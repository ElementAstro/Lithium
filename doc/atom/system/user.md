# User Information Library Documentation

## Function: GetUserGroups

- Retrieves the user groups associated with the current user.

- A vector of wide strings containing user groups.

```cpp
std::vector<std::wstring> userGroups = GetUserGroups();
// Expected output: {"group1", "group2", "group3"}
```

## Function: getUsername

- Retrieves the username of the current user.

- The username as a string.

```cpp
std::string username = getUsername();
// Expected output: "john_doe"
```

## Function: getHostname

- Retrieves the hostname of the system.

- The hostname as a string.

```cpp
std::string hostname = getHostname();
// Expected output: "mycomputer"
```

## Function: getUserId

- Retrieves the user ID of the current user.

- The user ID as an integer.

```cpp
int userId = getUserId();
// Expected output: 1001
```

## Function: getGroupId

- Retrieves the group ID of the current user.

- The group ID as an integer.

```cpp
int groupId = getGroupId();
// Expected output: 1001
```

## Function: getHomeDirectory

- Retrieves the user's profile directory.

- The user's profile directory as a string.

```cpp
std::string homeDirectory = getHomeDirectory();
// Expected output: "/home/john_doe"
```

## Function: getLoginShell

- Retrieves the login shell of the user.

- The login shell as a string.

```cpp
std::string loginShell = getLoginShell();
// Expected output: "/bin/bash"
```
