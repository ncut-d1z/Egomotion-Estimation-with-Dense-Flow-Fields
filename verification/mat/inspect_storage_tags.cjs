// 只读诊断：独立于 matio，读取当前单变量 MAT v5 的实数存储标签。
const fs=require('fs'),path=require('path'),zlib=require('zlib'),crypto=require('crypto');
const root=process.argv[2];
function requireFact(ok,message){if(!ok)throw new Error(message);}
function element(buffer,offset){
  requireFact(offset+8<=buffer.length,'truncated tag');
  const word=buffer.readUInt32LE(offset),small=word>>>16;
  const type=small?word&0xffff:word;
  const bytes=small||buffer.readUInt32LE(offset+4),start=offset+(small?4:8);
  requireFact(!small||small<=4,'invalid small tag');
  requireFact(start+bytes<=buffer.length,'truncated data');
  return {type,bytes,start,next:small?offset+8:start+Math.ceil(bytes/8)*8};
}
function* files(folder){
  for(const entry of fs.readdirSync(folder,{withFileTypes:true}).sort((a,b)=>a.name.localeCompare(b.name))){
    const item=path.join(folder,entry.name);
    if(entry.isDirectory())yield*files(item);
    else if(entry.isFile()&&entry.name.endsWith('.mat'))yield item;
  }
}
const counts={},records=[];
for(const file of files(root)){
  const raw=fs.readFileSync(file);
  requireFact(raw.length>=136&&raw.toString('ascii',126,128)==='IM'&&raw.readUInt16LE(124)===256,file+': not LE v5');
  const compressed=element(raw,128);
  requireFact(compressed.type===15&&compressed.start+compressed.bytes===raw.length,file+': not single compressed element');
  const inflated=zlib.inflateSync(raw.subarray(compressed.start,compressed.start+compressed.bytes),{maxOutputLength:1048576});
  const matrix=element(inflated,0);
  requireFact(matrix.type===14&&matrix.start+matrix.bytes===inflated.length,file+': matrix bounds');
  const flags=element(inflated,matrix.start),dims=element(inflated,flags.next);
  const name=element(inflated,dims.next),real=element(inflated,name.next);
  requireFact(flags.type===6&&flags.bytes===8&&dims.type===5&&dims.bytes===8&&name.type===1,file+': unsupported headers');
  const cls=inflated.readUInt32LE(flags.start)&255;
  const variable=inflated.toString('utf8',name.start,name.start+name.bytes);
  requireFact(variable===path.basename(file,'.mat'),file+': name mismatch');
  const key='class='+cls+', real_tag='+real.type;
  counts[key]=(counts[key]||0)+1;
  records.push({file:path.relative(root,file),sha256:crypto.createHash('sha256').update(raw).digest('hex'),
    variable,cls,realType:real.type,realBytes:real.bytes,dims:[inflated.readInt32LE(dims.start),inflated.readInt32LE(dims.start+4)]});
}
console.log(JSON.stringify({kind:'independent-readonly-storage-tag-diagnostic',count:records.length,counts,records},null,2));
