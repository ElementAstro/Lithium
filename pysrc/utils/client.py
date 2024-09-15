import asyncio
import json


class JSONTCPClient:
    def __init__(self, host='127.0.0.1', port=8888, max_retries=5):
        """
        Initialize the JSONTCPClient with the server's host, port, and maximum number of retries.

        Args:
            host (str): The server's hostname or IP address.
            port (int): The server's port number.
            max_retries (int): The maximum number of connection retries.
        """
        self.host = host
        self.port = port
        self.max_retries = max_retries
        self.reader = None
        self.writer = None

    async def connect(self):
        """
        Attempt to connect to the server with retry logic.

        Returns:
            bool: True if connected successfully, False otherwise.
        """
        retries = 0
        while retries < self.max_retries:
            try:
                self.reader, self.writer = await asyncio.open_connection(self.host, self.port)
                print(f'Connected to server at {self.host}:{self.port}')
                return True
            except (ConnectionRefusedError, asyncio.TimeoutError):
                retries += 1
                print(f'Retrying to connect ({retries}/{self.max_retries})...')
                await asyncio.sleep(2)
        print('Connection failed. Please check the server.')
        return False

    async def disconnect(self):
        """
        Disconnect from the server.
        """
        if self.writer:
            self.writer.close()
            await self.writer.wait_closed()
            self.writer = None
            print('Disconnected from server.')

    async def reconnect(self):
        """
        Attempt to reconnect to the server.
        """
        await self.disconnect()
        await self.connect()

    async def send_message(self, message):
        """
        Send a JSON message to the server.

        Args:
            message (dict): The message to send.
        """
        if self.writer is None:
            if not await self.connect():
                return

        try:
            request = json.dumps(message) + '\n'
            print(f'Sending: {request.strip()}')
            self.writer.write(request.encode())
            await self.writer.drain()

            response = await self.receive_message()
            print(f'Received: {response}')
        except Exception as e:
            print(f'Error: {e}')
            await self.reconnect()

    async def receive_message(self):
        """
        Receive a message from the server.

        Returns:
            str: The received message.
        """
        try:
            response = await self.reader.readline()
            return response.decode().strip()
        except Exception as e:
            print(f'Error receiving message: {e}')
            return None

    async def ping(self):
        """
        Send a ping message to the server to check its availability.

        Returns:
            bool: True if the server responds, False otherwise.
        """
        try:
            await self.send_message({"command": "ping"})
            response = await self.receive_message()
            return response == "pong"
        except Exception as e:
            print(f'Ping error: {e}')
            return False

    def set_max_retries(self, max_retries):
        """
        Set the maximum number of connection retries.

        Args:
            max_retries (int): The new maximum number of retries.
        """
        self.max_retries = max_retries

    async def close(self):
        """
        Close the connection to the server.
        """
        await self.disconnect()


async def main():
    client = JSONTCPClient(host='127.0.0.1', port=8888)

    commands = [
        {"command": "echo", "message": "Hello, Server!"},
        {"command": "run", "cmd": "echo 'Running command!'"},
        {"command": "run", "cmd": "ls -l"}
    ]

    for cmd in commands:
        await client.send_message(cmd)
        await asyncio.sleep(1)

    await client.close()

if __name__ == '__main__':
    asyncio.run(main())
