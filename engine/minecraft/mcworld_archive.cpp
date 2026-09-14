#include "minecraft/mcworld_archive.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <zlib.h>
namespace pvr::minecraft { namespace {
std::uint16_t u16(const std::vector<std::uint8_t>& b,std::size_t p){return std::uint16_t(b[p])|(std::uint16_t(b[p+1])<<8);}
std::uint32_t u32(const std::vector<std::uint8_t>& b,std::size_t p){return std::uint32_t(b[p])|(std::uint32_t(b[p+1])<<8)|(std::uint32_t(b[p+2])<<16)|(std::uint32_t(b[p+3])<<24);}
void err(std::vector<MinecraftDiagnostic>& d,const std::string&s,const std::string&m){d.push_back({MinecraftDiagnosticSeverity::Error,s,m,false});}
}
bool MinecraftArchiveReader::read(const std::filesystem::path& path,std::vector<MinecraftArchiveEntry>& entries,std::vector<MinecraftDiagnostic>& diagnostics){
 std::ifstream in(path,std::ios::binary); if(!in){err(diagnostics,path.string(),"cannot open archive");return false;} in.seekg(0,std::ios::end); auto end=in.tellg(); if(end<22){err(diagnostics,path.string(),"archive is too small");return false;} std::vector<std::uint8_t>b((std::size_t)end); in.seekg(0);in.read((char*)b.data(),end);if(!in){err(diagnostics,path.string(),"cannot read archive");return false;}
 std::size_t eocd=std::string::npos; std::size_t lower=b.size()>22+65535?b.size()-22-65535:0; for(std::size_t p=b.size()-22+1;p-- > lower;){if(p+4<=b.size()&&u32(b,p)==0x06054b50){eocd=p;break;}} if(eocd==std::string::npos){err(diagnostics,path.string(),"missing ZIP end-of-central-directory record");return false;}
 auto count=u16(b,eocd+10); auto cdSize=u32(b,eocd+12); auto cdOffset=u32(b,eocd+16); if(std::uint64_t(cdOffset)+cdSize>b.size()){err(diagnostics,path.string(),"central directory exceeds archive");return false;}
 std::size_t p=cdOffset; for(std::uint16_t i=0;i<count;++i){if(p+46>b.size()||u32(b,p)!=0x02014b50){err(diagnostics,path.string(),"invalid central directory entry");return false;} auto method=u16(b,p+10);auto compressed=u32(b,p+20);auto uncompressed=u32(b,p+24);auto nl=u16(b,p+28);auto el=u16(b,p+30);auto cl=u16(b,p+32);auto lo=u32(b,p+42); if(p+46ull+nl+el+cl>b.size()){err(diagnostics,path.string(),"central directory entry exceeds archive");return false;} std::string name((char*)b.data()+p+46,nl); if(name.empty()||name.front()=='/'||name.find("..")!=std::string::npos){err(diagnostics,name,"unsafe archive path");return false;} if(std::uint64_t(lo)+30>b.size()||u32(b,lo)!=0x04034b50){err(diagnostics,name,"invalid local file header");return false;} auto lnl=u16(b,lo+26);auto lel=u16(b,lo+28);std::size_t data=std::size_t(lo)+30ull+lnl+lel;if(std::uint64_t(data)+compressed>b.size()){err(diagnostics,name,"file data exceeds archive");return false;} std::vector<std::uint8_t>out(uncompressed); if(method==0){if(compressed!=uncompressed){err(diagnostics,name,"stored entry size mismatch");return false;}std::copy_n(b.begin()+data,compressed,out.begin());}else if(method==8){z_stream zs{};zs.next_in=(Bytef*)(b.data()+data);zs.avail_in=compressed;zs.next_out=(Bytef*)out.data();zs.avail_out=uncompressed;if(inflateInit2(&zs,-MAX_WBITS)!=Z_OK){err(diagnostics,name,"cannot initialize DEFLATE decoder");return false;}int rc=inflate(&zs,Z_FINISH);bool ok=rc==Z_STREAM_END&&zs.total_out==uncompressed;inflateEnd(&zs);if(!ok){err(diagnostics,name,"invalid DEFLATE payload");return false;}}else{err(diagnostics,name,"unsupported ZIP compression method");return false;}entries.push_back({std::move(name),std::move(out)});p+=46ull+nl+el+cl; }
 return true;
}
}
