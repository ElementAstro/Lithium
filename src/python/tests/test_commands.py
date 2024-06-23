import pytest
from app.dispatch import command_dispatcher

@pytest.mark.asyncio
async def test_hello_command():
    result = await command_dispatcher.execute_command("hello", "Alice", user_permissions=["basic"])
    assert result == "Hello, Alice!"

@pytest.mark.asyncio
async def test_hello_command_without_permission():
    with pytest.raises(PermissionError):
        await command_dispatcher.execute_command("hello", "Alice", user_permissions=[])

@pytest.mark.asyncio
async def test_command_cooldown():
    await command_dispatcher.execute_command("hello", "Alice", user_permissions=["basic"])
    with pytest.raises(ValueError):
        await command_dispatcher.execute_command("hello", "Alice", user_permissions=["basic"])

@pytest.mark.asyncio
async def test_list_commands():
    commands = command_dispatcher.list_commands()
    assert "hello" in commands
    assert commands["hello"] == "Say hello"