"""Refresh RevC numeric data from the frozen handoff and hash-bound native exports."""
import argparse
import csv
import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def read_csv(path):
    with path.open(encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream))


def number(value):
    return round(float(value), 6)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exports", type=Path, help="native geometry and Konnect audit evidence directory")
    args = parser.parse_args()
    archive = ROOT / "baseline"
    schema = read_json(archive / "matrix_schema.json")
    rows = read_json(archive / "matrix_numeric.json")
    parts = read_json(archive / "component_dictionary.json")
    manifest = read_json(args.exports / "Overview_Manifest.json")
    audit = read_json(args.exports / "Audit_Source.json")
    assert manifest["board_sha256"] == audit["board_sha256"], "Mismatched geometry and validation sources"
    assert manifest["board_sha256"] != schema["source"]["1"].split()[-1], "The old board cannot be the current source"
    footprints = sorted(read_csv(args.exports / "Component_Position_Map.csv"), key=lambda x: x["reference"])
    pads = read_csv(args.exports / "Pad_Coordinate_Map.csv")
    inventory = {part["reference"]: part for part in read_csv(args.exports / "Part_Inventory.csv")}
    references = [part["reference"] for part in footprints]
    assert len(set(references)) == len(footprints) == manifest["footprints"]
    assert set(references) == inventory.keys()
    assert len(pads) == manifest["pads"]
    previous = ROOT / "component_dictionary.json"
    prior = read_json(previous) if previous.exists() else {}
    ids = {entry["reference"]: int(key) for key, entry in prior.items() if entry.get("record_type") == "saved_pcb_anchor"}
    next_id = max(ids.values(), default=1999) + 1
    old = {part["name"]: int(key) for key, part in parts.items() if "kicad_at" in part}
    pad_count = Counter(pad["reference"] for pad in pads)
    schema["schema_version"] = "1.1"
    schema["class"]["11"] = "saved PCB footprint anchor"
    schema["status"]["7"] = "observed in current saved PCB source"
    schema["source"]["6"] = "native KiCad export of the saved 116-footprint repair lineage"
    schema["source"]["7"] = "RevC assembly study, candidate only"
    schema["saved_pcb_sha256"] = manifest["board_sha256"]
    schema["saved_pcb_counts"] = {key: manifest[key] for key in ("footprints", "pads", "vias", "layers")}
    schema["saved_pcb_counts"]["zones"] = len(manifest["zones_hidden_only_in_export"])
    proof_path = args.exports / "Continuation_Connectivity.json"
    if proof_path.exists():
        proof = read_json(proof_path)
        assert proof["board_sha256"] == manifest["board_sha256"], "Stale layer/connectivity proof"
        schema["saved_layer_contract"] = {
            "F.Cu": "signals and local supply escapes",
            "In1.Cu": "continuous main GND; independent ISO_GND",
            "In2.Cu": "3V3 / 5V_CORE zones, remaining GND and independent ISO_GND",
            "B.Cu": "signals and local supply escapes",
        }
        schema["layer_contract_evidence"] = "evidence/Continuation_Connectivity.json"
    schema["semantics"] += " Class 11 records have native KiCad rotation; XYZ of as-installed geometry remains unverified."
    schema["snapshot_counts_scope"] = "older uploaded board only"
    for part in footprints:
        ref = part["reference"]
        if ref not in ids:
            ids[ref] = next_id
            next_id += 1
        matrix_id = ids[ref]
        x, y, angle = (number(part[key]) for key in ("x_mm", "y_mm", "rotation_deg"))
        mount = {"F.Cu": 1, "B.Cu": 4}[part["side"]]
        rows.append([matrix_id, 11, 7, 6, mount, round(x - 150, 6), round(100 - y, 6), None,
                     None, None, None, angle, 0, None, None, None, None, None])
        parts[str(matrix_id)] = {
            "name": ref, "reference": ref, "record_type": "saved_pcb_anchor", "side": part["side"],
            "kicad_at": [x, y, angle], "pad_count": pad_count[ref], "old_matrix_id": old.get(ref),
            "value": inventory[ref]["value"], "footprint": inventory[ref]["footprint"],
            "registered_3d_models": inventory[ref]["model_references"].split(" | ") if inventory[ref]["model_references"] else [],
            "source_sha256": manifest["board_sha256"],
            "detail": "保存板真实封装锚点。角度为 KiCad 原生值，背面 XY 不镜像。安装 Z 及外形未知。",
        }
    active = set(references)
    population_path = args.exports / "Population_Inventory.json"
    if population_path.exists():
        population = read_json(population_path)
        assert population["board_sha256"] == manifest["board_sha256"], "Stale population inventory"
        for footprint in population["footprints"]:
            ref = footprint["reference"]
            parts[str(ids[ref])]["dnp"] = footprint["dnp"]
            schematic = population["schematic"].get(ref)
            if schematic:
                parts[str(ids[ref])]["schematic_dnp"] = schematic["dnp"]
    for ref, matrix_id in ids.items():
        if ref in active:
            continue
        entry = dict(prior[str(matrix_id)])
        entry["detail"] = "封装位号已从当前保存板清单移除，保留 ID 供变更追溯。"
        parts[str(matrix_id)] = entry
        rows.append([matrix_id, 11, 6, 6, 0, *([None] * 13)])
    for candidate in read_json(ROOT / "assembly_candidates.json"):
        row, entry = candidate["matrix_row"], candidate["dictionary_entry"]
        assert str(row[0]) not in parts
        rows.append(row)
        parts[str(row[0])] = entry
    for design_id, ref in {166: "JML1", 167: "JMR1", 180: "SWJ1"}.items():
        if ref in active:
            parts[str(design_id)]["saved_anchor_id"] = ids[ref]
    current_descriptions = {
        "180": "SLLB510100 三路输入。保存网表接 GPIO38/40/41，共用 GND。上拨录音、中按系统静音、下拨 Agent 通知中心，实物上下朝向待机构核对。",
        "210": "RevC 采用 ADS1015 采集四路 OH49E，地址 0x48。实际模拟路径、供电和 I²C 以当前保存证据为准。",
        "211": "四颗 WS2812C-4020-H2。GPIO10 数据经 U22 TXU0101 电平转换，GPIO42 控制 RGB_EN 与 U21 受控电源，默认下拉关闭。动态失效保护及功耗待实测。",
    }
    for design_id, description in current_descriptions.items():
        parts[design_id]["detail"] = description
    bindings_path = ROOT / "electrical_bindings.json"
    if bindings_path.exists():
        bindings = read_json(bindings_path)
        assert bindings["schematic_sha256"] == audit["schematic_sha256"], "Stale electrical bindings"
        for design_id, binding in bindings["components"].items():
            assert design_id in parts, f"Unknown design ID {design_id}"
            parts[design_id]["electrical_binding"] = binding
    rows.sort(key=lambda row: row[0])
    assert len({row[0] for row in rows}) == len(rows) == len(parts)
    assert {str(row[0]) for row in rows} == parts.keys()
    assert all(len(row) == len(schema["columns"]) and all(x is None or type(x) in (int, float) for x in row) for row in rows)
    for name, value in (("matrix_schema", schema), ("matrix_numeric", rows), ("component_dictionary", parts)):
        write_json(ROOT / f"{name}.json", value)
    with (ROOT / "matrix_numeric.csv").open("w", encoding="utf-8", newline="") as stream:
        csv.writer(stream, lineterminator="\n").writerows(rows)
    print(f"{len(rows)} rows, {len(footprints)} verified saved-board anchors")


if __name__ == "__main__":
    main()
