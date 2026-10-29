# HMQ Wire Protocol

## 1. Frame format

The header is 16 bytes long.

| Field | Size | Description |
|---|---:|---|
| Magic | 4 bytes | ASCII `HMQ1` |
| Version | 2 bytes | Protocol version; currently `1` |
| Message type | 2 bytes | Identifies the operation |
| Request ID | 4 bytes | Associates a response with a request |
| Payload length | 4 bytes | Number of payload bytes |
| Payload | Variable | Operation-specific data |

All multi-byte header integers use big-endian byte order.

The maximum payload size is 1 MiB (1,048,576 bytes).

A complete frame consists of the 16-byte header followed by exactly the number of payload bytes declared in the header.

## 2. Message types

| Name | Value | Purpose |
|---|---:|---|
| `PING` | `0x0001` | Check whether the broker responds |
| `PING_RESPONSE` | `0x8001` | Successful response to a ping |
| `ERROR` | `0xFFFF` | Indicates a request or processing error |
| `PUBLISH` | `0x0002` | Append a message to the broker's log |
| `PUBLISH_ACK` | `0x8002` | Return the assigned message offset |
| `CONSUME` | `0x0003` | Retrieve a message by offset |
| `CONSUME_RESPONSE` | `0x8003` | Return the requested message and its offset |

Response message types use the high bit to distinguish them from their corresponding request types. Existing numeric values must not change.

## 3. Proposed operation payloads

All offsets are unsigned 64-bit integers encoded in big-endian byte order.

### PUBLISH

**Request payload:** The raw message bytes.

The broker appends the message to its persistent log and assigns it an offset.

**Successful response:** `PUBLISH_ACK`

| Field | Size |
|---|---:|
| Assigned offset | 8 bytes |

The response uses the same request ID as the request.

### CONSUME

**Request payload:** The requested offset.

| Field | Size |
|---|---:|
| Offset | 8 bytes |

The broker retrieves the message at that offset without deleting it.

**Successful response:** `CONSUME_RESPONSE`

| Field | Size |
|---|---:|
| Message offset | 8 bytes |
| Message content | Remaining payload bytes |

The response uses the same request ID as the request.

### ERROR

The current implementation uses a UTF-8 text payload to describe an error. A stable, machine-readable error-code format has not yet been finalized.

The broker should return an error for invalid requests, unavailable offsets, and storage failures where appropriate. The precise error payload format must be specified before the publish and consume handlers are implemented.
