var path = require('path');

var fs = require('fs');
let cpu=JSON.parse(fs.readFileSync(path.dirname(__filename)+'\\Opcodes.json'));

unp=cpu.unprefixed;

const hex=Array(256).fill(0).map((a,b)=>"0x"+("0"+b.toString(16)).substr(-2).toUpperCase());

let nim=hex.reduce((a,b)=>{
    a[unp[b].mnemonic]=(a[unp[b].mnemonic]||0)+1;
    return a;
},{});

let nims=Object.keys(nim).sort();

function ntor(n){
    if(!n.immediate){
        return `p<${n.name}>`;
    }else if(n.name.match(/^([ABCDEHL]|(AF)|(BC)|(DE)|(HL)|(SP)|(PC))$/)){
        return `r<${n.name}>`;
    }else if(n.name.match(/^..H$/)){
        return `i8<0x${n.name[0]+n.name[1]}>`;
    }else if(n.name.match(/^(r8)$/)){
        return `s8`;
    }else{
        return n.name;
    }
}

let code=hex.filter(a=>!unp[a].mnemonic.match("ILLEGAL")).map(a=>
    `      case ${a}:exec<${unp[a].mnemonic}${
        unp[a].operands.length>=1?","+ntor(unp[a].operands[0]):""
    }${
        unp[a].operands.length>=2?","+ntor(unp[a].operands[1]):""
    }> (${unp[a].cycles[0]}); break;${unp[a].cycles[1]?"// "+unp[a].cycles[1]:""}`
).join("\n");
console.log(code);
fs.writeFileSync(path.dirname(__filename)+'\\opscode.cc',code);

