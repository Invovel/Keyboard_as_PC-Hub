// R2: millimetres; design +Y is rear. Projection flips Y, never the layout.
// Concave key outlines are extruded directly: no convex hull on a key.
const {Cam, fit, proj, poly, open, rrect, fillet, mk, pointer, disposer, register, tween, tset, tval, tdone} = HL;
const SLOPE = Math.tan(5 * Math.PI / 180);
const CIRCLE = (r, x=0, y=0) => Array.from({length:96}, (_,i) => [x+r*Math.cos(i*Math.PI/48), y+r*Math.sin(i*Math.PI/48)]);
const CCW = q => q.reduce((s,a,i)=>{const b=q[(i+1)%q.length];return s+a[0]*b[1]-b[0]*a[1];},0)<0?q.reverse():q;
const RECT = (x0,y0,x1,y1,r=1) => CCW(rrect(x0,y0,x1,y1,r,8).map(p=>[p.u,p.v]));
function keyShape(sx,sy) {
  const r=34.5, a=Math.asin(.75/r), b=Math.asin(31/r), w=sx<0?45:51;
  const p=[[Math.sqrt(r*r-.75*.75),.75],[w,.75],[w,31],[Math.sqrt(r*r-31*31),31]];
  for(let i=1;i<=48;i++){const t=b+(a-b)*i/48;p.push([r*Math.cos(t),r*Math.sin(t)]);}
  const rounded=fillet(p,p.map((_,i)=>i===1||i===2?1:0),5);
  const q=rounded.filter((a,i)=>{const b=rounded[(i+rounded.length-1)%rounded.length];return Math.hypot(a[0]-b[0],a[1]-b[1])>1e-8;}).map(([x,y])=>[sx*x,sy*y]);
  return sx*sy<0?q.reverse():q;
}
const KEYS=[[-1,1],[1,1],[1,-1],[-1,-1]].map(([x,y])=>keyShape(x,y));
const CENTERS=[[-36,22],[36,22],[36,-22],[-36,-22]];
function mount({stage,svg,read},value) {
  const bag=disposer(), parts=[], C=Cam(45,.5,1.38);
  fit(C,[[-48,34,0],[197,-34,0],[197,75,0],[-48,-34,160],[54,34,160]],200,154);
  const raw=proj(C), P=(x,y,z)=>raw(x,-y,z), root=mk('g',{},svg);
  let gap=value, active=-1, pinned=-1, last='', dirty=true;
  const main=tween(0), macro=tween(0), strip=tween(0);
  function add(name,shape,z0,z1,level,options={}) {
    const g=mk('g',{'data-part':name},root);
    const obj={name,shape,z0,z1,level,holes:[],kind:'main',tilt:false,...options,g,
      side:mk('path',{class:'fo'},g),rim:mk('path',{class:'nf sil'},g),
      face:mk('path',{class:'sil','fill-rule':'evenodd'},g),marks:mk('path',{class:'nf lo'},g)};
    parts.push(obj);return obj;
  }
  add('底壳',RECT(-48,-34,54,34,4),0,2.4,0);
  add('壳体侧壁',RECT(-48,-34,54,34,4),2.4,17.6,0,{tilt:true,holes:[RECT(-45.6,-31.6,51.6,31.6,1.6)],flatBase:true});
  for(const [x,y] of [[-41,-27],[47,-27],[-41,27],[47,27]])add('支柱',CIRCLE(2.5,x,y),2.4,6.4,0);
  add('主PCB',RECT(-45,-31,51,31,1.2),6.4,8,1,{marks2:[RECT(-20,-12,0,5),RECT(4,-10,20,8),RECT(-28,9,-13,23)]});
  for(const [i,x] of [-35,-15,5,22.5,37.5].entries()) {
    const w=i<3?15:10;
    add(i<3?'USB-A':'USB-C',RECT(x-w/2,17.5,x+w/2,33.2,.8),8,15,1,{socket:{axis:'y',at:33.3,x,w:w-2}});
  }
  add('FAST',RECT(-47,-6,-33,5,.8),8,12.5,1,{socket:{axis:'x',at:-47.1,y:-.5,w:8}});
  add('上壳',RECT(-48,-34,54,34,4),17.6,20,2,{tilt:true,holes:[CIRCLE(25),...KEYS]});
  CENTERS.forEach(([x,y],i)=>{
    add('感应板'+(i+1),RECT(x-7,y-7,x+7,y+7),16,17.6,2.5,{tilt:true,marks2:[RECT(x-2,y-2,x+2,y+2,.5)]});
    add('软键'+(i+1),KEYS[i],18,23,4.5,{tilt:true});
  });
  for(const a of [30,150,270]) {
    const x=19*Math.cos(a*Math.PI/180),y=19*Math.sin(a*Math.PI/180);
    add('弹性支撑',CIRCLE(2.2,x,y),15.5,20,3,{tilt:true});
  }
  add('按压限位',RECT(-3,-3,3,3,.6),12.5,15.5,3,{tilt:true});
  add('防转导向',RECT(-2,21,2,26,.6),16,22,3,{tilt:true});
  add('屏幕托架',CIRCLE(24.5),20,22.7,3,{tilt:true,holes:[CIRCLE(20)]});
  add('屏幕',CIRCLE(22.16),22.7,26.2,4,{tilt:true,marks2:[CIRCLE(19.08)]});
  add('外旋转环',CIRCLE(30),20,28,4.5,{tilt:true,holes:[CIRCLE(25)],upperHole:20.88});
  add('KBD支架',RECT(-24,-41,-10,-30,2),3,10,0);
  add('Type-C公头',RECT(-22,-47,-12,-40,.8),6,9.8,0);
  add('STOP',RECT(32,-35,42,-32,1),10,14,0);
  add('五键模块',RECT(55,-34,167,34,4),0,20,0,{kind:'macro',tilt:true,flatBase:true});
  for(let i=0;i<5;i++)add('宏键'+(i+1),RECT(66+i*19.05-8,12,66+i*19.05+8,28,2),21,29,0,{kind:'macro',tilt:true,marks2:[RECT(66+i*19.05-6,14,66+i*19.05+6,26,1.5)]});
  add('触控条',RECT(59,-30,163,-8,3),20,28,0,{kind:'strip',tilt:true,marks2:[RECT(64,-25,158,-13,2)]});
  for(const y of [-9,9]) {
    add('主机磁吸座',RECT(51.5,y-2,54,y+2,.8),10,14,0);
    add('附件磁吸座',RECT(55,y-2,57.5,y+2,.8),10,14,0,{kind:'macro'});
  }
  for(const y of [-4,-2,0,2,4])add('弹簧触点',RECT(53,y-.45,54.6,y+.45,.3),11,12,0);
  add('磁吸连接面',RECT(54,-12,54.4,12,.2),9,15,0,{connector:true});
  for(const x of [68,154])add('条形定位座',RECT(x-2,-24,x+2,-16,1),20,21,0,{kind:'macro',tilt:true});
  for(const x of [105,108,111,114,117])add('触控条触点',CIRCLE(.6,x,-19),20,20.6,0,{kind:'macro',tilt:true});
  function drawPart(p,e,m,s) {
    const dx=p.kind==='main'?0:m*gap*1.25,dy=p.kind==='strip'?-s*gap:0,dz=p.kind==='main'?e*gap*p.level:p.kind==='strip'?s*gap*.5:0;
    const z=(y,h)=>h+(p.tilt?(y+34)*SLOPE:0)+dz;
    const at=(q,h)=>P(q[0]+dx,q[1]+dy,p.flatBase&&h===p.z0?h+dz:z(q[1],h)), top=p.shape.map(q=>at(q,p.z1));
    let side='',rim='';
    const contour=(q,inward=false)=>{
      const visible=i=>{const a=q[(i+q.length)%q.length],b=q[(i+1+q.length)%q.length];return (b[0]-a[0]+b[1]-a[1])*(inward?-1:1)>0;};
      for(let i=0;i<q.length;i++){
        const a=q[i],b=q[(i+1)%q.length],f=(b[0]-a[0]+b[1]-a[1])*(inward?-1:1);
        if(f<=0)continue;
        side+=poly(CCW([at(a,p.z0),at(b,p.z0),at(b,p.z1),at(a,p.z1)]));
        if(inward)continue;
        rim+=open([at(a,p.z0),at(b,p.z0)]);
        if(!visible(i-1))rim+=open([at(a,p.z0),at(a,p.z1)]);
        if(!visible(i+1))rim+=open([at(b,p.z0),at(b,p.z1)]);
      }
    };
    p.holes.forEach(h=>contour(h,true));contour(p.shape);
    p.side.setAttribute('d',side);p.rim.setAttribute('d',rim);
    const holes=p.upperHole?[CIRCLE(p.upperHole)]:p.holes;
    p.face.setAttribute('d',poly(top)+holes.map(h=>poly(h.map(q=>at(q,p.z1)))).join(''));
    let marks=(p.marks2||[]).map(q=>poly(q.map(a=>at(a,p.z1+.02)))).join('');
    if(p.socket){const k=p.socket,h=RECT(-k.w/2,-1.4,k.w/2,1.4,.65);marks+=poly(h.map(([u,v])=>k.axis==='y'?P(k.x+u,k.at,p.z1-3+v+dz):P(k.at,k.y+u,p.z1-2.2+v+dz)));}
    if(p.connector){const face=q=>poly(q.map(([y,z])=>P(54.4,y,z)));p.face.setAttribute('d',face(RECT(-12,9,12,15,1)));p.side.setAttribute('d','');p.rim.setAttribute('d','');marks=[CIRCLE(1.5,-8,12),CIRCLE(1.5,8,12),...[-4,-2,0,2,4].map(y=>CIRCLE(.45,y,12))].map(face).join('');}
    p.marks.setAttribute('d',marks);
    const hi=active<0?p.name==='外旋转环':active===0?p.name==='主PCB':active===1?p.name==='屏幕托架':active===2?p.name==='磁吸连接面':p.name==='触控条';
    p.face.classList.toggle('hi',hi);
    p.depth=z(0,p.z1)*1000+(p.shape.reduce((v,q)=>v+q[0]-q[1],0)/p.shape.length)+dx-dy;
    if(p.connector)p.depth=21500;
  }
  // Static projected rest footprints: movement can never change the hit target.
  function inside([x,y],q){let yes=false;for(let i=0,j=q.length-1;i<q.length;j=i++){const [a,b]=q[i],[c,d]=q[j];if((b>y)!==(d>y)&&x<(c-a)*(y-b)/(d-b)+a)yes=!yes;}return yes;}
  const target=(shape,h)=>shape.map(([x,y])=>P(x,y,h+(y+34)*SLOPE));
  const hits=[target(RECT(-48,-34,54,34,4),20),target(CIRCLE(24.5),28),target(RECT(55,-34,167,34,4),20),target(RECT(59,-30,163,-8,3),28)];
  function hit(q){for(const i of [1,3,2,0])if(inside(q,hits[i]))return i;return -1;}
  function choose(a){
    if(a===active)return;active=a;const now=performance.now();
    tset(main,a===0||a===1?1:0,now,0);tset(macro,a===2?1:0,now,0);tset(strip,a===3?1:0,now,0);
    read.textContent=['主机分层','屏幕托架','磁吸五键','磁吸触控条'][a]||'rest';dirty=true;loop.wake();
  }
  const loop=register(stage,(_dt,now)=>{
    const e=tval(main,now),m=tval(macro,now),s=tval(strip,now),signature=[e,m,s,gap,active].join('/');
    if(dirty||signature!==last){parts.forEach(p=>drawPart(p,e,m,s));parts.slice().sort((a,b)=>a.depth-b.depth).forEach(p=>root.append(p.g));last=signature;dirty=false;}
    return !tdone(main,now)||!tdone(macro,now)||!tdone(strip,now);
  });
  bag.add(loop.unregister);
  bag.add(pointer(stage,{move:q=>{if(pinned<0)choose(hit(q));},leave:()=>{if(pinned<0)choose(-1);},down:q=>{const a=hit(q);pinned=a===pinned?-1:a;choose(pinned);}}));
  bag.add(()=>svg.replaceChildren());read.textContent='rest';loop.wake();
  return {set(v){gap=v;dirty=true;loop.wake();},destroy:bag.dispose};
}
hairline({
  name:'keyboard-hub',
  means:'左小右长，圆心不动。悬停拆看主机与磁吸附件，点击固定、再次点击或点空白复位；滑块为展示分离距离（mm）。尺寸为装配候选，尚未完成制造验证。',
  rules:[1,3,4,6,7,8,9,10], range:[8,16,24],
  tour:[[77,172],[127,166],[257,206],[217,232],null], mount,
});
