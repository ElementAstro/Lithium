import asyncio
import json
from typing import Any, Dict, Optional, Callable, List, Tuple
import threading
import signal
from loguru import logger

class JSONTCPClient:
    def __init__(
        self,
        host: str = '127.0.0.1',
        port: int = 8888,
        max_retries: int = 5,
        connect_timeout: int = 10,
        read_timeout: int = 10,
        keep_connected: bool = True,
        on_connect: Optional[Callable[[], None]] = None,
        on_disconnect: Optional[Callable[[], None]] = None,
        on_reconnect: Optional[Callable[[], None]] = None,
    ):
        self.host = host
        self.port = port
        self.max_retries = max_retries
        self.connect_timeout = connect_timeout
        self.read_timeout = read_timeout
        self.keep_connected = keep_connected
        self.reader: Optional[asyncio.StreamReader] = None
        self.writer: Optional[asyncio.StreamWriter] = None
        self.on_connect = on_connect
        self.on_disconnect = on_disconnect
        self.on_reconnect = on_reconnect
        self.message_handlers: Dict[str, Callable[[Dict[str, Any]], None]] = {}
        self._connected_event = asyncio.Event()
        self._running_event = threading.Event()
        self.loop = asyncio.get_event_loop()

    async def connect(self) -> bool:
        retries = 0
        while retries < self.max_retries:
            try:
                self.reader, self.writer = await asyncio.wait_for(
                    asyncio.open_connection(self.host, self.port),
                    timeout=self.connect_timeout,
                )
                logger.info(f'Connected to server at {self.host}:{self.port}')
                if self.on_connect:
                    self.on_connect()
                self._connected_event.set()
                return True
            except (ConnectionRefusedError, asyncio.TimeoutError) as e:
                retries += 1
                logger.warning(f'Retrying to connect ({retries}/{self.max_retries})... Error: {e}')
                await asyncio.sleep(2)
        logger.error('Connection failed. Please check the server.')
        return False

    async def send_message(self, message: Dict[str, Any], handler: Optional[Callable[[Dict[str, Any]], None]] = None) -> Optional[Dict[str, Any]]:
        await self._connected_event.wait()

        try:
            request = json.dumps(message) + '\n'
            logger.debug(f'Sending: {request.strip()}')
            self.writer.write(request.encode())
            await self.writer.drain()

            response = await asyncio.wait_for(self.reader.readline(), timeout=self.read_timeout)
            response_str = response.decode().strip()
            logger.debug(f'Received: {response_str}')
            response_data = json.loads(response_str)

            if handler:
                handler(response_data)
            elif 'command' in message and message['command'] in self.message_handlers:
                self.message_handlers[message['command']](response_data)

            return response_data
        except asyncio.TimeoutError:
            logger.error('Read timeout')
            await self.handle_disconnect()
        except Exception as e:
            logger.error(f'Error: {e}')
            await self.handle_disconnect()
        return None

    async def send_messages(self, messages: List[Tuple[Dict[str, Any], Optional[Callable[[Dict[str, Any]], None]]]]) -> List[Optional[Dict[str, Any]]]:
        results = []
        for message, handler in messages:
            result = await self.send_message(message, handler)
            results.append(result)
            await asyncio.sleep(1)
        return results

    def register_handler(self, command: str, handler: Callable[[Dict[str, Any]], None]) -> None:
        self.message_handlers[command] = handler

    async def handle_disconnect(self) -> None:
        await self.close()
        if self.keep_connected:
            logger.info('Reconnecting...')
            await self.connect()

    async def close(self) -> None:
        if self.writer:
            logger.info('Closing connection')
            self.writer.close()
            await self.writer.wait_closed()
            self.writer = None
            if self.on_disconnect:
                self.on_disconnect()
        self._connected_event.clear()

    async def keep_connection(self) -> None:
        self._running_event.set()
        while self.keep_connected and self._running_event.is_set():
            if not self._connected_event.is_set():
                await self.connect()
            await asyncio.sleep(1)

    async def __aenter__(self) -> 'JSONTCPClient':
        if not await self.connect():
            raise ConnectionError('Failed to connect to server.')
        return self

    async def __aexit__(self, exc_type, exc, tb) -> None:
        await self.close()

    def stop(self) -> None:
        self._running_event.clear()


def run_client_in_thread(client: JSONTCPClient) -> None:
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(client.keep_connection())
    loop.close()


def signal_handler(client: JSONTCPClient, client_thread: threading.Thread) -> Callable[[int, Any], None]:
    def handle_signal(sig, frame):
        logger.info('Received exit signal. Stopping client...')
        client.keep_connected = False
        client.stop()
        client_thread.join()
        logger.info('Client stopped. Exiting...')
        exit(0)
    return handle_signal


async def main() -> None:
    def on_connect():
        logger.info('Connected to server.')

    def on_disconnect():
        logger.info('Disconnected from server.')

    def on_reconnect():
        logger.info('Reconnected to server.')

    def echo_handler(response: Dict[str, Any]):
        logger.info(f'Echo handler received: {response}')

    client = JSONTCPClient(
        host='127.0.0.1',
        port=4400,
        on_connect=on_connect,
        on_disconnect=on_disconnect,
        on_reconnect=on_reconnect
    )

    client.register_handler('echo', echo_handler)

    commands = [
        ({"command": "echo", "message": "Hello, Server!"}, None),
        ({"command": "run", "cmd": "echo 'Running command!'"}, None),
        ({"command": "run", "cmd": "ls -l"}, None)
    ]

    client_thread = threading.Thread(target=run_client_in_thread, args=(client,))
    client_thread.start()

    signal.signal(signal.SIGINT, signal_handler(client, client_thread))

    async with client:
        responses = await client.send_messages(commands)
        for response in responses:
            logger.info(f'Response: {response}')

    client.keep_connected = False
    client_thread.join()


if __name__ == '__main__':
    asyncio.run(main())
