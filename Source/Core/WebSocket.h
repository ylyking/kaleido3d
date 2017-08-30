#ifndef __WebSocket_H__
#define __WebSocket_H__

#include "Os.h"

namespace net
{
    enum WebSocketFrameType {
        ERROR_FRAME=0xFF00,
        INCOMPLETE_FRAME=0xFE00,

        OPENING_FRAME=0x3300,
        CLOSING_FRAME=0x3400,

        INCOMPLETE_TEXT_FRAME=0x01,
        INCOMPLETE_BINARY_FRAME=0x02,

        TEXT_FRAME=0x81,
        BINARY_FRAME=0x82,

        PING_FRAME=0x19,
        PONG_FRAME=0x1A
    };

	class WebSocketImpl;

    class K3D_API WebSocket : public Os::Socket
    {
        friend class WebSocketImpl;
    public:
							WebSocket();
        virtual				~WebSocket();

		Os::SocketHandle	Accept(Os::IPv4Address & ipAddr) override;
		uint64				Receive(Os::SocketHandle reomte, void * pData, uint32 recvLen) override;
		uint64				Send(Os::SocketHandle remote, const char * pData, uint32 sendLen) override;

    protected:
        WebSocketImpl*      d;
    };
}

#endif