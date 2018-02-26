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

	std::map<SRV_CMDS, std::wstring> CmdMap = {
		{ CMD_STOP, L"stop" },
		{ CMD_RESET, L"reset" }
	};


  std::wcout << CmdMap[CMD_STOP];
  std::wcout << L", complete\n";

  return EXIT_SUCCESS;
}
