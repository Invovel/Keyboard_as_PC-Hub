// R3: 228x76 mm, Nape-inspired four-key symmetry. +Y is rear.
// Concave key outlines are extruded directly: no convex hull on a key.
const {Cam, fit, proj, poly, open, rrect, fillet, mk, pointer, disposer, register, tween, tset, tval, tdone} = HL;
const SLOPE = Math.tan(5 * Math.PI / 180);
const CIRCLE = (r, x=0, y=0) => Array.from({length:96}, (_,i) => [x+r*Math.cos(i*Math.PI/48), y+r*Math.sin(i*Math.PI/48)]);
const CCW = q => q.reduce((s,a,i)=>{const b=q[(i+1)%q.length];return s+a[0]*b[1]-b[0]*a[1];},0)<0?q.reverse():q;
const RECT = (x0,y0,x1,y1,r=1) => CCW(rrect(x0,y0,x1,y1,r,8).map(p=>[p.u,p.v]));
function keyShape(sx,sy) {
  const r=34.5, a=Math.asin(1/r), b=Math.asin(31/r), w=108;
  const p=[[Math.sqrt(r*r-1),1],[w,1],[w,31],[Math.sqrt(r*r-31*31),31]];
  for(let i=1;i<=48;i++){const t=b+(a-b)*i/48;p.push([r*Math.cos(t),r*Math.sin(t)]);}
  const rounded=fillet(p,p.map((_,i)=>i===1||i===2?1:0),5);
  const q=rounded.filter((a,i)=>{const b=rounded[(i+rounded.length-1)%rounded.length];return Math.hypot(a[0]-b[0],a[1]-b[1])>1e-8;}).map(([x,y])=>[sx*x,sy*y]);
  return sx*sy<0?q.reverse():q;
}
const KEYS=[[-1,1],[1,1],[1,-1],[-1,-1]].map(([x,y])=>keyShape(x,y));
const CENTERS=[[-65,18],[65,18],[65,-18],[-65,-18]];
function mount({stage,svg,read},value) {
  const bag=disposer(), parts=[], C=Cam(45,.5,1);
  fit(C,[[-114,38,0],[257,38,0],[257,-62,0],[-114,38,160],[114,-38,160]],200,154);
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
  add('底壳',RECT(-114,-38,114,38,5),0,2.4,0);
  add('壳体侧壁',RECT(-114,-38,114,38,5),2.4,17.6,0,{tilt:true,holes:[RECT(-111.6,-35.6,111.6,35.6,2.6)],flatBase:true});
  for(const [x,y] of [[-104,-28],[104,-28],[-104,28],[104,28]])add('支柱',CIRCLE(3,x,y),2.4,6.4,0);
  add('主PCB',RECT(-108,-32,108,32,1.2),6.4,8,1,{marks2:[RECT(-101,22,-50,26),RECT(-101,13,-76,17),RECT(57,19,73,24),RECT(47,-23,61,-18)]});
  for(const [i,x] of [-91,-64,64,91,105].entries()) {
    const w=i<3?15:10;
    add(i<3?'USB-A':'USB-C',RECT(x-w/2,21.5,x+w/2,37.2,.8),8,15,1,{socket:{axis:'y',at:37.3,x,w:w-2}});
  }
  add('FAST',RECT(-113,12.5,-99,21.5,.8),8,12.5,1,{socket:{axis:'x',at:-113.1,y:17,w:8}});
  add('麦克风声腔',RECT(-95,-35,-81,-25,2),5,12,1,{marks2:[CIRCLE(.8,-88,-30)]});
  add('扬声器腔',RECT(82,-32,102,-16,3),3,11,1,{marks2:[CIRCLE(6,92,-24)]});
  add('上壳',RECT(-114,-38,114,38,5),17.6,20,2,{tilt:true,holes:[CIRCLE(25),...KEYS]});
  CENTERS.forEach(([x,y],i)=>{
    add('感应板'+(i+1),RECT(x-7,y-7,x+7,y+7),16,17.6,2.5,{tilt:true,marks2:[RECT(x-2,y-2,x+2,y+2,.5)]});
    add('软键'+(i+1),KEYS[i],18,20.3,4.5,{tilt:true});
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
  add('KBD支架',RECT(-20,-45,-6,-34,2),3,10,0);
  add('Type-C公头',RECT(-18,-51,-8,-44,.8),6,9.8,0);
  add('STOP',RECT(92,-39,102,-36,1),10,14,0);
  add('五键模块',RECT(115,-38,227,38,5),0,20,0,{kind:'macro',tilt:true,flatBase:true});
  for(let i=0;i<5;i++)add('宏键'+(i+1),RECT(133+i*19.05-8,9,133+i*19.05+8,25,2),21,29,0,{kind:'macro',tilt:true,marks2:[RECT(133+i*19.05-6,11,133+i*19.05+6,23,1.5)]});
  add('触控条',RECT(119,-35,223,-13,3),20,28,0,{kind:'strip',tilt:true,marks2:[RECT(124,-30,218,-18,2)]});
  for(const y of [-27,27]) {
    for(const x of [-114,111.5])add('主机磁吸座',RECT(x,y-2.5,x+2.5,y+2.5,.8),10,15,0);
    add('附件磁吸座',RECT(115,y-2.5,117.5,y+2.5,.8),10,15,0,{kind:'macro'});
  }
  for(const y of [-8.75,-6.25,-3.75,-1.25,1.25,3.75,6.25,8.75])for(const x of [-114,113])add('弹簧触点',RECT(x,y-.45,x+1.6,y+.45,.3),11,12,0);
  add('磁吸连接面',RECT(114,-31,114.4,31,.2),8,16,0,{connector:true});
  for(const x of [128,214])add('条形定位座',RECT(x-2,-29,x+2,-21,1),20,21,0,{kind:'macro',tilt:true});
  for(const x of [165,168,171,174,177])add('触控条触点',CIRCLE(.6,x,-24),20,20.6,0,{kind:'macro',tilt:true});
  function drawPart(p,e,m,s) {
    const dx=p.kind==='main'?0:m*gap*1.25,dy=p.kind==='strip'?-s*gap:0,dz=p.kind==='main'?e*gap*p.level:p.kind==='strip'?s*gap*.5:0;
    const z=(y,h)=>h+(p.tilt?(y+38)*SLOPE:0)+dz;
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
    if(p.connector){const face=q=>poly(q.map(([y,z])=>P(114.4,y,z)));p.face.setAttribute('d',face(RECT(-31,8,31,16,1)));p.side.setAttribute('d','');p.rim.setAttribute('d','');marks=[CIRCLE(2.5,-27,12),CIRCLE(2.5,27,12),...[-8.75,-6.25,-3.75,-1.25,1.25,3.75,6.25,8.75].map(y=>CIRCLE(.45,y,12))].map(face).join('');}
    p.marks.setAttribute('d',marks);
    const hi=active<0?p.name==='外旋转环':active===0?p.name==='主PCB':active===1?p.name==='屏幕托架':active===2?p.name==='磁吸连接面':p.name==='触控条';
    p.face.classList.toggle('hi',hi);
    p.depth=z(0,p.z1)*1000+(p.shape.reduce((v,q)=>v+q[0]-q[1],0)/p.shape.length)+dx-dy;
    if(p.connector)p.depth=21500;
  }
  // Static projected rest footprints: movement can never change the hit target.
  function inside([x,y],q){let yes=false;for(let i=0,j=q.length-1;i<q.length;j=i++){const [a,b]=q[i],[c,d]=q[j];if((b>y)!==(d>y)&&x<(c-a)*(y-b)/(d-b)+a)yes=!yes;}return yes;}
  const target=(shape,h)=>shape.map(([x,y])=>P(x,y,h+(y+38)*SLOPE));
  const hits=[target(RECT(-114,-38,114,38,5),20),target(CIRCLE(24.5),28),target(RECT(115,-38,227,38,5),20),target(RECT(119,-35,223,-13,3),28)];
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
  means:'桌面供电与控制底座，键盘通过独立公头接入。悬停拆看主体与磁吸模块、点击固定；滑块控制展示分离距离（mm）。供电、Hub与结构仍为设计候选。',
  rules:[1,3,4,6,7,8,9,10], range:[8,16,24],
  tour:[[84,136],[141,157],[262,225],[245,228],null], mount,
});
