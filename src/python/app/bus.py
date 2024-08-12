import asyncio
import inspect
from loguru import logger
from typing import Any, Callable, Deque, Dict, List, Optional, Type
from collections import deque
from dataclasses import dataclass, field

@dataclass
class Message:
    pass

class Priority:
    LOW = 10
    MEDIUM = 5
    HIGH = 1

@dataclass
class Handler:
    func: Callable[[Message], Any]
    priority: int = Priority.MEDIUM

class MessageBus:
    def __init__(self):
        self._subscribers: Dict[Type[Message], List[Handler]] = {}
        self._filters: List[Callable[[Message], bool]] = []
        self._cache: Deque[Message] = deque(maxlen=100)  # 缓存最近的100条消息
        self._queue: asyncio.Queue[Message] = asyncio.Queue()
        self._running: bool = False

    def subscribe(self, message_type: Type[Message], handler: Callable[[Message], Any], priority: int = Priority.MEDIUM) -> None:
        if message_type not in self._subscribers:
            self._subscribers[message_type] = []
        self._subscribers[message_type].append(Handler(handler, priority))
        self._subscribers[message_type].sort(key=lambda h: h.priority)

    def unsubscribe(self, message_type: Type[Message], handler: Callable[[Message], Any]) -> None:
        if message_type in self._subscribers:
            self._subscribers[message_type] = [h for h in self._subscribers[message_type] if h.func != handler]
            if not self._subscribers[message_type]:
                del self._subscribers[message_type]

    def add_filter(self, filter_func: Callable[[Message], bool]) -> None:
        self._filters.append(filter_func)

    def remove_filter(self, filter_func: Callable[[Message], bool]) -> None:
        self._filters.remove(filter_func)

    async def publish(self, message: Message, broadcast: bool = True) -> None:
        await self._queue.put(message)
        self._cache.append(message)

        if broadcast:
            await self._broadcast(message)

    async def _process_queue(self) -> None:
        while self._running:
            message = await self._queue.get()
            if not all(filter_func(message) for filter_func in self._filters):
                logger.info(f"Message filtered: {message}")
                continue

            if type(message) in self._subscribers:
                handlers = self._subscribers[type(message)]
                for handler in handlers:
                    await self._handle_message(handler, message)

    async def _handle_message(self, handler: Handler, message: Message) -> None:
        try:
            if inspect.iscoroutinefunction(handler.func):
                await handler.func(message)
            else:
                handler.func(message)
        except Exception as e:
            logger.error(f"Error handling message {message} with handler {handler.func}: {e}")

    async def _broadcast(self, message: Message) -> None:
        logger.info(f"Broadcasting message: {message}")
        for message_type, handlers in self._subscribers.items():
            if message_type != type(message):
                for handler in handlers:
                    await self._handle_message(handler, message)

    def get_recent_messages(self, limit: int = 10) -> List[Message]:
        return list(self._cache)[-limit:]

    def get_subscribers(self, message_type: Type[Message]) -> List[Handler]:
        return self._subscribers.get(message_type, [])

    def get_filters(self) -> List[Callable[[Message], bool]]:
        return self._filters

    def start(self) -> None:
        if not self._running:
            self._running = True
            asyncio.create_task(self._process_queue())

    def stop(self) -> None:
        self._running = False

# 示例用法
async def main():
    bus = MessageBus()

    # 定义消息类型
    @dataclass
    class MyMessage(Message):
        content: str

    # 定义处理器
    async def async_handler(msg: MyMessage):
        logger.info(f"Async handler received: {msg.content}")

    def sync_handler(msg: MyMessage):
        logger.info(f"Sync handler received: {msg.content}")

    # 订阅消息
    bus.subscribe(MyMessage, async_handler, priority=Priority.HIGH)
    bus.subscribe(MyMessage, sync_handler, priority=Priority.LOW)

    # 添加过滤器
    def filter_func(msg: MyMessage) -> bool:
        return "filter" not in msg.content

    bus.add_filter(filter_func)

    # 启动消息总线
    bus.start()

    # 发布消息
    await bus.publish(MyMessage("Hello World!"))
    await bus.publish(MyMessage("This message will be filtered"), broadcast=False)

    # 获取最近的消息
    recent_messages = bus.get_recent_messages()
    logger.info(f"Recent messages: {recent_messages}")

    # 关闭消息总线
    bus.stop()

# 运行示例
if __name__ == "__main__":
    asyncio.run(main())
