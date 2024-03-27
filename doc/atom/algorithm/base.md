# Base Encoding & Decoding Functions

## encodeBase16

**Brief:** Encodes a vector of unsigned characters into a Base16 string.

This function takes a vector of unsigned characters and encodes it into a Base16 string representation. Base16 encoding, also known as hexadecimal encoding, represents each byte in the input data as a pair of hexadecimal digits (0-9, A-F).

- **Parameters:**

  - `data`: The vector of unsigned characters to be encoded.

- **Return:**
  - The Base16 encoded string.

## decodeBase16

**Brief:** Decodes a Base16 string into a vector of unsigned characters.

This function takes a Base16 encoded string and decodes it into a vector of unsigned characters. Base16 encoding, also known as hexadecimal encoding, represents each byte in the input data as a pair of hexadecimal digits (0-9, A-F).

- **Parameters:**

  - `data`: The Base16 encoded string to be decoded.

- **Return:**
  - The decoded vector of unsigned characters.

## encodeBase32

**Brief:** Encodes a string to Base32.

- **Parameters:**

  - `input`: The string to encode.

- **Return:**
  - The encoded string.

## decodeBase32

**Brief:** Decodes a Base32 string.

- **Parameters:**

  - `input`: The string to decode.

- **Return:**
  - The decoded string.

## base64Encode

**Brief:** Base64 encoding function.

- **Parameters:**

  - `bytes_to_encode`: Data to be encoded.

- **Return:**
  - Encoded string.

## base64Decode

**Brief:** Base64 decoding function.

- **Parameters:**

  - `encoded_string`: String to decode.

- **Return:**
  - Decoded data as a vector of unsigned characters.

## encodeBase85

**Brief:** Encodes a vector of unsigned characters into a Base85 string.

This function takes a vector of unsigned characters and encodes it into a Base85 string representation. Base85 encoding is a binary-to-text encoding scheme that encodes 4 bytes into 5 ASCII characters.

- **Parameters:**

  - `data`: The vector of unsigned characters to be encoded.

- **Return:**
  - The Base85 encoded string.

## decodeBase85

**Brief:** Decodes a Base85 string into a vector of unsigned characters.

This function takes a Base85 encoded string and decodes it into a vector of unsigned characters. Base85 encoding is a binary-to-text encoding scheme that encodes 4 bytes into 5 ASCII characters.

- **Parameters:**

  - `data`: The Base85 encoded string to be decoded.

- **Return:**
  - The decoded vector of unsigned characters.
