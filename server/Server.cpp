#include <cstdio>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

int Run(void);
int InitializeServer(void);
int FinalizeServer(void);

#define TIME_PERIOD_MSEC (1)
#define LISTEN_SOCKET_ADDR (L"0.0.0.0")
#define LISTEN_SOCKET_TCPPORT (9000)
#define MAX_BACKLOG (65535)
#define RECV_SIZE (1024)
#define SEND_SIZE (1024)

SOCKET ghListenSocket;
int gTCPNoDelay = 1;

int wmain(const int argc, const wchar_t* argv[])
{
	InitializeServer();
	{
		Run();
	}
	FinalizeServer();

	return 0;
}

int Run(void)
{
	SOCKET clientSocket;
	SOCKADDR_IN clientAddrInet;
	int addrLength;
	int retNoDelay;
	int retRecv;
	int retSend;
	char* paRecvBuf;
	char* paSendBuf;

	addrLength = sizeof(clientAddrInet);
	paRecvBuf = nullptr;
	paSendBuf = nullptr;

	for (;;)
	{
		clientSocket = accept(ghListenSocket, reinterpret_cast<SOCKADDR*>(&clientAddrInet), &addrLength);
		if (clientSocket == INVALID_SOCKET)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				fwprintf(stderr, L"[%s:%5u] accept() WSAGetLastError: %d\t%s:L#%d\n",
					TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
				break;
			}
		}
		else
		{
			if (gTCPNoDelay != 0)
			{
				retNoDelay = setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&gTCPNoDelay), sizeof(gTCPNoDelay));
				if (retNoDelay == SOCKET_ERROR)
				{
					fwprintf(stderr, L"[%s:%5u] setsockopt(TCP_NODELAY) WSAGetLastError: %d\t%s:L%d\n",
						TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
					break;
				}
			}

			paRecvBuf = (char*)malloc(RECV_SIZE);
			paSendBuf = (char*)malloc(SEND_SIZE);
			if (paRecvBuf == nullptr)
			{
				break;
			}
			if (paSendBuf == nullptr)
			{
				break;
			}

			for (;;)
			{
				retRecv = recv(clientSocket, paRecvBuf, RECV_SIZE, 0);
				if (retRecv == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						fwprintf(stderr, L"[%s:%5u] recv() WSAGetLastError: %d\t%s:L#%d\n",
							TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
						break;
					}
				}
				else if (retRecv == 0)
				{
					break;
				}
				else
				{
					wprintf(L"Recv(%d): ", retRecv);
					fwrite(paRecvBuf, 1, retRecv, stdout);
					wprintf(L"\n");

					memcpy(paSendBuf, paRecvBuf, retRecv);
					retSend = send(clientSocket, paSendBuf, retRecv, 0);

					if (retSend == SOCKET_ERROR)
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							fwprintf(stderr, L"[%s:%5u] send() WSAGetLastError: %d\t%s:L#%d\n",
								TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
							break;
						}
					}
					else
					{
						wprintf(L"\tSend(%d): ", retSend);
						fwrite(paSendBuf, 1, retSend, stdout);
						wprintf(L"\n");
					}
				}
			}
		}
	}

	if (paRecvBuf != nullptr)
	{
		free(paRecvBuf);
	}

	return 0;
}

int InitializeServer(void)
{
	MMRESULT retTimeBeginPeriod;
	retTimeBeginPeriod = timeBeginPeriod(TIME_PERIOD_MSEC);
	if (retTimeBeginPeriod != TIMERR_NOERROR)
	{
		fwprintf(stderr, L"[%s:%5u] timeBeginPeriod() GetLastError: %d\t%s:L#%d\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), GetLastError(), TEXT(__FILE__), __LINE__);
		return retTimeBeginPeriod;
	}

	int retWSAStartup;
	WSADATA wsaData;
	retWSAStartup = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retWSAStartup != 0)
	{
		fwprintf(stderr, L"[%s:%5u] WSAStartup() WSAGetLastError: %d\t%s:L#%d\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return retWSAStartup;
	}

	ghListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ghListenSocket == INVALID_SOCKET)
	{
		fwprintf(stderr, L"[%s:%5u] socket() WSAGetLastError: %d\t%s:L#%d\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return WSAGetLastError();
	}

	u_long nonBlockingMode = 1;
	int retIoctlSocket;
	retIoctlSocket = ioctlsocket(ghListenSocket, FIONBIO, &nonBlockingMode);
	if (retIoctlSocket == SOCKET_ERROR)
	{
		fwprintf(stderr, L"[%s:%5u] ioctlsocket() WSAGetLastError: %d\n%s:L#%d\n\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return WSAGetLastError();
	}

	INT retInetPtonW;
	SOCKADDR_IN serverAddrInet;
	memset(&serverAddrInet, 0, sizeof(serverAddrInet));
	serverAddrInet.sin_family = AF_INET;
	serverAddrInet.sin_port = htons(LISTEN_SOCKET_TCPPORT);
	retInetPtonW = InetPtonW(AF_INET, LISTEN_SOCKET_ADDR, &serverAddrInet.sin_addr);
	if (retInetPtonW != 1)
	{
		fwprintf(stderr, L"[%s:%5u] InetPtonW() WSAGetLastError: %d\n%s:L#%d\n\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return WSAGetLastError();
	}

	LINGER optLinger;
	int retSetSockOptLinger;
	optLinger.l_onoff = 1;
	optLinger.l_linger = 0;
	retSetSockOptLinger = setsockopt(ghListenSocket, SOL_SOCKET, SO_LINGER, (char*)&optLinger, sizeof(optLinger));
	if (retSetSockOptLinger == SOCKET_ERROR)
	{
		fwprintf(stderr, L"[%s:%5u] setsockopt() WSAGetLastError: %d\n%s:L%d\n\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return WSAGetLastError();
	}

	int retBind;
	retBind = bind(ghListenSocket, reinterpret_cast<SOCKADDR*>(&serverAddrInet), sizeof(serverAddrInet));
	if (retBind == SOCKET_ERROR)
	{
		fwprintf(stderr, L"[%s:%5u] bind() WSAGetLastError: %d\t%s:L#%d\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return retBind;
	}

	int maxBacklog = SOMAXCONN_HINT(MAX_BACKLOG);
	int retListen;
	retListen = listen(ghListenSocket, maxBacklog);
	if (retListen == SOCKET_ERROR)
	{
		fwprintf(stderr, L"[%s:%5u] listen(%zu, %d) WSAGetLastError: %d\n%s:L#%d\n\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), ghListenSocket, maxBacklog, WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return WSAGetLastError();
	}

	return 0;
}

int FinalizeServer(void)
{
	int retCloseSocket;
	retCloseSocket = closesocket(ghListenSocket);
	if (retCloseSocket == SOCKET_ERROR)
	{
		fwprintf(stderr, L"[%s:%5u] closesocket() WSAGetLastError: %d\n%s:L#%d\n\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return WSAGetLastError();
	}

	int retWSACleanup;
	retWSACleanup = WSACleanup();
	if (retWSACleanup != 0)
	{
		fwprintf(stderr, L"[%s:%5u] WSACleanup() WSAGetLastError: %d\t%s:L#%d\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), WSAGetLastError(), TEXT(__FILE__), __LINE__);
		return retWSACleanup;
	}

	MMRESULT retTimeEndPeriod;
	retTimeEndPeriod = timeEndPeriod(TIME_PERIOD_MSEC);
	if (retTimeEndPeriod != TIMERR_NOERROR)
	{
		fwprintf(stderr, L"[%s:%5u] timeEndPeriod(%d) TIMERR_NOERROR\n%s:L#%d\n\n",
			TEXT(__FUNCTION__), GetCurrentThreadId(), TIME_PERIOD_MSEC, TEXT(__FILE__), __LINE__);
		return retTimeEndPeriod;
	}

	return 0;
}