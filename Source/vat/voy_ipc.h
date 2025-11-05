//
//	Rexxless-Voyager-IPC
//

#define VCMD_GOTOURL 1

#define VIPCNAME "webbrowser_ipc"

struct voyager_msg {
	struct Message m;
	ULONG cmd;
	char *parms;
	ULONG reserved[ 14 ];
};
