<%@ WebHandler Language="C#" Class="EchoWebSocket" %>

using System;
using System.Web;
using System.Net.WebSockets;
using System.Web.WebSockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

public class EchoWebSocket : IHttpHandler {
        private const int MaxBufferSize = 64 * 1024;

        public void ProcessRequest (HttpContext context)
        {
            try
            {
                context.AcceptWebSocketRequest(async wsContext =>
                {
                    byte[] receiveBuffer = new byte[MaxBufferSize];
                    ArraySegment<byte> buffer = new ArraySegment<byte>(receiveBuffer);
                    WebSocket socket = wsContext.WebSocket;
                    string userString;

                    // Stay in loop while websocket is open
                    while (socket.State == WebSocketState.Open)
                    {
                        WebSocketReceiveResult receiveResult = await socket.ReceiveAsync(buffer, CancellationToken.None);

                        if (receiveResult.MessageType == WebSocketMessageType.Close)
                        {
                            // Echo back code and reason strings if possible
                            if (receiveResult.CloseStatus == WebSocketCloseStatus.Empty)
                            {
                                await socket.CloseAsync(WebSocketCloseStatus.Empty, null, CancellationToken.None);
                            }
                            else
                            {
                                await socket.CloseAsync(
                                    receiveResult.CloseStatus.GetValueOrDefault(),
                                    receiveResult.CloseStatusDescription,
                                    CancellationToken.None);
                            }
                            return;
                        }

                        int offset = receiveResult.Count;

                        while (receiveResult.EndOfMessage == false)
                        {
                            receiveResult = await socket.ReceiveAsync(new ArraySegment<byte>(receiveBuffer, offset, MaxBufferSize - offset), CancellationToken.None);
                            offset += receiveResult.Count;
                        }

                        if (receiveResult.MessageType == WebSocketMessageType.Text)
                        {
                            string cmdString = Encoding.UTF8.GetString(receiveBuffer, 0, offset);
                            userString = cmdString;
                            if (userString == ".close")
                            {
                                await socket.CloseAsync(WebSocketCloseStatus.EndpointUnavailable, "Win8 server says goodbye", CancellationToken.None);
                            }
                            else if (userString == ".abort")
                            {
                                socket.Abort();
                            }
                            else
                            {
                                userString = "You said: \"" + userString + "\"";

                                ArraySegment<byte> outputBuffer = new ArraySegment<byte>(Encoding.UTF8.GetBytes(userString));

                                await socket.SendAsync(outputBuffer, WebSocketMessageType.Text, true, CancellationToken.None);
                            }
                        }
                        else if (receiveResult.MessageType == WebSocketMessageType.Binary)
                        {
                            ArraySegment<byte> outputBuffer = new ArraySegment<byte>(receiveBuffer, 0, receiveResult.Count);

                            await socket.SendAsync(outputBuffer, WebSocketMessageType.Binary, true, CancellationToken.None);
                        }
                    }
                });
            }
            catch (Exception ex)
            {
                context.Response.StatusCode = 500;
                context.Response.StatusDescription = ex.Message;
                context.Response.End();
            }
        }

        public bool IsReusable
        {
            get
            {
                return false;
            }
        }
}
