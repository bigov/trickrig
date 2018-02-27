#include <iostream>
#include <string>
#include <vector>
#include <map>

int main()
{
  enum SRV_CMDS {
    CMD_STOP,
    CMD_RESET,
    CMDS_LIST_END
  };

  std::map<SRV_CMDS, const wchar_t*> CmdMap0 = {
    { CMD_STOP, L"stop" },
    { CMD_RESET, L"reset" }
  };

  std::map<SRV_CMDS, const wchar_t*> CmdMap1 = {
    { CMD_STOP, L"stops" },
    { CMD_RESET, L"reset" }
  };

  if(CmdMap0[CMD_RESET] == CmdMap1[CMD_RESET])
  {
    std::wcout << L"CmdMap[CMD_RESET] equal \n";
  } else {
    std::wcout << L"CmdMap[CMD_RESET] NOT equal \n";
  }

  if(CmdMap0[CMD_STOP] == CmdMap1[CMD_STOP])
  {
    std::wcout << L"CmdMap[CMD_STOP] equal \n";
  } else {
    std::wcout << L"CmdMap[CMD_STOP] NOT equal \n";
  }

  return EXIT_SUCCESS;
}

