            class millennium{static sharedjscontext_exec(c){const t=new WebSocket(atob('d3M6Ly9sb2NhbGhvc3Q6MzI0Mg==')),r=()=>t.send(JSON.stringify({type:6,content:c}));return new Promise((e,n)=>{t.onopen=r,t.onmessage=t=>(e(JSON.parse(t.data)),t.target.close()),t.onerror=n,t.onclose=()=>n(new Error)})}}window.millennium=millennium;