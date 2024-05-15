import asyncio
import json
import subprocess
import logging
from typing import Dict, Any

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

class JSONTCPServer:
    """
    An asynchronous TCP server that handles JSON-formatted commands and sends back JSON-formatted responses.
    """

    def __init__(self, host='127.0.0.1', port=8888, max_clients=10):
        self.host = host
        self.port = port
        self.max_clients = max_clients
        self.active_clients = 0
        self.semaphore = asyncio.Semaphore(max_clients)  # Limit concurrent connections

    async def handle_client(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        """
        Handles a single client connection.
        """
        client_addr = writer.get_extra_info('peername')
        logging.info(f'New connection from {client_addr}')
        self.active_clients += 1

        try:
            while True:
                data = await reader.readline()
                if not data:
                    break

                message = data.decode().strip()
                logging.debug(f'Received {message} from {client_addr}')

                try:
                    json_data = json.loads(message)
                    response = await self.process_command(json_data)
                except json.JSONDecodeError as e:
                    response = {"status": "error", "message": str(e)}

                await self.send_response(writer, response)

        except asyncio.CancelledError:
            logging.warning(f'Connection with {client_addr} was cancelled')
        except Exception as e:
            logging.error(f'Error handling client {client_addr}: {e}')
        finally:
            logging.info(f'Closing connection with {client_addr}')
            self.active_clients -= 1
            writer.close()
            await writer.wait_closed()

    async def process_command(self, json_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Processes the received JSON command and returns a JSON response.
        """
        command = json_data.get("command")
        if command == "echo":
            return self.handle_echo(json_data)
        elif command == "run":
            return await self.handle_run(json_data)
        else:
            return {"status": "error", "message": "Unknown command"}

    def handle_echo(self, json_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handles the "echo" command.
        """
        message = json_data.get("message", "")
        return {"status": "success", "response": message}

    async def handle_run(self, json_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Handles the "run" command, executing a shell command.
        """
        cmd = json_data.get("cmd", "")
        try:
            # Use asyncio.create_subprocess_shell for better async handling
            process = await asyncio.create_subprocess_shell(
                cmd,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            stdout, stderr = await process.communicate()
            return {
                "status": "success",
                "stdout": stdout.decode().strip(),
                "stderr": stderr.decode().strip(),
                "returncode": process.returncode
            }
        except Exception as e:
            return {"status": "error", "message": str(e)}

    async def send_response(self, writer: asyncio.StreamWriter, response: Dict[str, Any]):
        """
        Sends a JSON-formatted response to the client.
        """
        response_data = json.dumps(response) + '\n'
        writer.write(response_data.encode())
        await writer.drain()

    async def start_server(self):
        """
        Starts the TCP server and listens for client connections.
        """
        server = await asyncio.start_server(
            self.handle_client,
            self.host,
            self.port
        )
        addr = server.sockets[0].getsockname()
        logging.info(f'Serving on {addr}')

        async with server:
            await server.serve_forever()

async def main():
    server = JSONTCPServer(host='127.0.0.1', port=8888, max_clients=10)
    await server.start_server()

if __name__ == '__main__':
    asyncio.run(main())