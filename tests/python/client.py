import asyncio
import json

class JSONTCPClient:
    def __init__(self, host='127.0.0.1', port=8888, max_retries=5):
        self.host = host
        self.port = port
        self.max_retries = max_retries
        self.reader = None
        self.writer = None

    async def connect(self):
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

    async def send_message(self, message):
        if self.writer is None:
            if not await self.connect():
                return
        
        try:
            request = json.dumps(message) + '\n'
            print(f'Sending: {request.strip()}')
            self.writer.write(request.encode())
            await self.writer.drain()
            
            response = await self.reader.readline()
            print(f'Received: {response.decode().strip()}')
        except Exception as e:
            print(f'Error: {e}')
            self.writer.close()
            await self.writer.wait_closed()
            self.writer = None

    async def close(self):
        if self.writer:
            self.writer.close()
            await self.writer.wait_closed()

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