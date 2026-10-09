// Additional acceptance checks for the generated, self-contained HTML.
// Run after hairline look.mjs. HAIRLINE_LOOK_CACHE points at its Playwright cache.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const {pathToFileURL} = require('node:url');
const cache = process.env.HAIRLINE_LOOK_CACHE || path.join(process.env.LOCALAPPDATA, 'hairline-look');
const {chromium} = require(path.join(cache, 'node_modules/playwright-core'));
const source = fs.readFileSync(path.join(__dirname, 'keyboard-hub.js'), 'utf8');
const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
(async () => {
  const browser = await chromium.launch({channel:'chrome', headless:true});
  try {
    const context = await browser.newContext({viewport:{width:1000,height:1000},colorScheme:'light'});
    const page = await context.newPage(), errors=[];
    page.on('pageerror',e=>errors.push(e.message));
    page.on('console',m=>{if(['error','warning'].includes(m.type()))errors.push(m.text());});
    await page.addInitScript(() => {
      let hl, call;
      Object.defineProperty(window,'HL',{configurable:true,get:()=>hl,set:v=>{hl={...v,proj:C=>(window.testP=v.proj(C))};}});
      Object.defineProperty(window,'hairline',{configurable:true,get:()=>call,set:v=>{call=f=>{window.testFigure=f; const mount=f.mount;f.mount=(...args)=>(window.testHandle=mount(...args));v(f);};}});
    });
    const url=pathToFileURL(path.join(__dirname,'hairline-keyboard-hub.html')).href;
    await page.goto(url);await pause(900);
    const geometry=await page.evaluate(src=>{
      const prefix=src.slice(0,src.indexOf('function mount'));
      return new Function(prefix+';return {keys:KEYS,centers:CENTERS};')();
    },source);
    const area=q=>Math.abs(q.reduce((s,a,i)=>{const b=q[(i+1)%q.length];return s+a[0]*b[1]-b[0]*a[1];},0)/2);
    const areas=geometry.keys.map(area);
    assert.ok(Math.abs(areas[0]-areas[3])<1e-6 && Math.abs(areas[1]-areas[2])<1e-6,'same-side keys mirror');
    assert.ok(areas[1]/areas[0]>1.2 && areas[1]/areas[0]<1.5,'area ratio');
    assert.deepEqual(geometry.centers,[[-36,22],[36,22],[36,-22],[-36,-22]]);
    for(const key of geometry.keys){
      assert.ok(key.every(([x,y])=>Number.isFinite(x+y)&&Math.hypot(x,y)>34.49),'concave arc remains outside ring clearance');
      const turns=key.map((p,i)=>{const a=key[(i+key.length-1)%key.length],b=key[(i+1)%key.length];return (p[0]-a[0])*(b[1]-p[1])-(p[1]-a[1])*(b[0]-p[0]);});
      assert.ok(turns.some(v=>v<-.000001)&&turns.some(v=>v>.000001),'concavity retained');
    }
    const at=async(x,y,z)=>page.evaluate(([x,y,z])=>window.testP(x,-y,z),[x,y,z]);
    const point=async(q,type='pointermove')=>page.evaluate(([q,type])=>{
      const el=document.querySelector('#stage'),r=el.getBoundingClientRect();
      el.dispatchEvent(new PointerEvent(type,{pointerType:'mouse',pointerId:1,bubbles:true,clientX:r.left+q[0]/400*r.width,clientY:r.top+q[1]/320*r.height}));
    },[q,type]);
    const read=()=>page.locator('#read').textContent();
    const bbox=name=>page.locator(`[data-part="${name}"]`).first().evaluate(el=>{const b=el.getBBox();return {x:b.x,y:b.y,w:b.width,h:b.height};});
    const frame=async()=>{const b=await page.locator('#stage svg').evaluate(el=>{const b=el.getBBox();return [b.x,b.y,b.x+b.width,b.y+b.height];});assert.ok(b[0]>0&&b[1]>0&&b[2]<400&&b[3]<320,`frame ${b}`);};
    const restScreen=await bbox('屏幕'), restSensors=await Promise.all([1,2,3,4].map(i=>bbox('感应板'+i)));
    const main=await at(-32,-20,21.2),screen=await at(0,0,31),macro=await at(111,22,27.9),strip=await at(111,-19,29.3);
    console.log('viewBox targets',JSON.stringify({main,screen,macro,strip}));
    await point(main);await pause(850);assert.equal(await read(),'主机分层');await frame();
    assert.ok(Math.abs((await bbox('屏幕')).x-restScreen.x)<.01,'screen X fixed in explosion');
    for(let i=0;i<4;i++)assert.ok(Math.abs((await bbox('感应板'+(i+1))).x-restSensors[i].x)<.01,'sensor X fixed');
    await point(main,'pointerdown');await point([399,319]);await pause(850);assert.equal(await read(),'主机分层','click pins');
    await point(main,'pointerdown');await pause(850);assert.equal(await read(),'rest','same click resets');
    await point(screen);await pause(850);assert.equal(await read(),'屏幕托架');
    await point(screen,'pointerdown');await point([399,319],'pointerdown');await pause(850);assert.equal(await read(),'rest','blank click resets');
    const macroRest=await bbox('五键模块'),stripRest=await bbox('触控条');
    await point(macro);await pause(850);assert.equal(await read(),'磁吸五键');assert.ok((await bbox('五键模块')).x>macroRest.x+10);await frame();
    await page.locator('.plate').screenshot({path:path.join(__dirname,'hairline-keyboard-hub-magnetic.png')});
    await point(strip);await pause(850);assert.equal(await read(),'磁吸触控条');assert.ok((await bbox('触控条')).x<stripRest.x-5);await frame();
    await page.locator('.plate').screenshot({path:path.join(__dirname,'hairline-keyboard-hub-strip.png')});
    for(const value of [0,1])for(const q of [main,screen,macro,strip]){
      await page.locator('#intensity').evaluate((el,v)=>{el.value=String(v);el.dispatchEvent(new Event('input',{bubbles:true}));},value);
      await point(q);await pause(850);await frame();
    }
    const tour=await page.evaluate(()=>window.testFigure.tour), tourRead=[];
    for(const q of tour){await point(q||[399,319]);await pause(850);tourRead.push(await read());}
    assert.deepEqual(tourRead,['主机分层','屏幕托架','磁吸五键','磁吸触控条','rest'],'tour targets');
    await page.locator('#play').click();assert.equal(await page.locator('#play').getAttribute('aria-pressed'),'true');await pause(1700);
    assert.notEqual(await read(),'rest','play produces a response');await page.locator('#play').click();
    await page.emulateMedia({reducedMotion:'reduce'});await point(screen);await pause(250);await frame();
    const requests=[];page.on('request',r=>requests.push(r.url()));await page.reload();await pause(900);assert.ok(requests.every(u=>u.startsWith('file:')),'offline artifact');
    assert.deepEqual(errors,[]);
    await page.evaluate(()=>window.testHandle.destroy());assert.equal(await page.locator('#stage svg').evaluate(el=>el.childElementCount),0,'destroy empties svg');
    console.log('PASS: geometry, fixed centers, pin/reset, both magnetic modules, all slider extremes, tour, play, reduced motion, offline, disposal. Areas:',areas.map(v=>v.toFixed(2)).join(', '));
  } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
