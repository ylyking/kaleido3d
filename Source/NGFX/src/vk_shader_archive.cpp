#include <Kaleido3D.h>
#include "vulkan_glslang.h"
#include <Core/Os.h>
#include <unordered_map>
#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

class CommandUtil
{
public:
  CommandUtil(int argc, const char** argv)
  {
    MainArg = argv[0];
    int sId = 1;
    while (sId < argc)
    {
      if (std::string("-o") == (argv[sId]))
      {
        sId++;
        ArgValMap["-o"] = argv[sId];
      }
      else if (std::string("-c") == (argv[sId]))
      {
      }
      else if (std::string("-I") == (argv[sId]))
      {
        sId++;
        ArgValMap["-I"] = argv[sId];
      }
      else
      {
        ArgValMap["-c"] = argv[sId];
      }
      sId++;
    }
  }
  
  std::string GetArg(std::string const& name) const
  {
    return ArgValMap.at(name);
  }

private:
  std::string MainArg;
  std::unordered_map<std::string, std::string> ArgValMap;
};

/*
 * vkc -c [[ -o [output file] ]] {shader files}
 */
int main(int argc, const char* argv[])
{
  CommandUtil Util(argc, argv);
  FunctionMap DataBlob;
  std::string Error;

  Os::File SrcFile;
  if(!SrcFile.Open(Util.GetArg("-c").c_str(), IORead))
  {
    return -1;
  }
  uint64 SzFile = SrcFile.GetSize();
  char* SrcData = new char[SzFile + 1];
  SrcFile.Read(SrcData, SzFile);
  SrcData[SzFile] = 0;
  CompileFromSource(ngfx::CompileOption(), SrcData, DataBlob, Error);
  delete SrcData;
  SerializeLibrary(DataBlob, Util.GetArg("-o").c_str());
  return 0;
}