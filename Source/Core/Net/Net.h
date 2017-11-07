#ifndef __WebSocket_H__
#define __WebSocket_H__

namespace k3d
{
    namespace net
    {
        class WebSocketImpl;

        class K3D_CORE_API WebSocket : public os::Socket
        {
            friend class WebSocketImpl;
        public:
            WebSocket();
            virtual				~WebSocket();

            os::Socket*	        Accept(os::IpAddress const& Ip) override;
            U64				    Receive(void * pData, U64 recvLen) override;
            U64				    Send(const char * pData, U64 sendLen) override;

        protected:
            WebSocketImpl*      d;
        };

        class K3D_CORE_API HttpRequest
        {
        public:
            HttpRequest();
        };

        class K3D_CORE_API HttpClient
        {
        public:
            HttpClient();
            ~HttpClient();


        };


    }
}

#endif