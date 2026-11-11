# iPhone Shortcut

The Shortcuts app speaks HTTP, not MQTT, so it talks to the Adafruit IO REST API.

## "Wake PC"

1. **Get Contents of URL**
   - URL: `https://io.adafruit.com/api/v2/YOUR_USERNAME/feeds/pc-command/data`
   - Method: **POST**
   - Headers: `X-AIO-Key` = your Adafruit IO key (no empty header rows)
   - Request Body: **JSON**, field `value` (Text) = `wake` (must match `WAKE_COMMAND`)
2. **Show Notification**: "Command sent"
3. **Wait** 60 seconds
4. **Get Contents of URL**
   - URL: `https://io.adafruit.com/api/v2/YOUR_USERNAME/feeds/pc-status/data/last`
   - Method: **GET**, same `X-AIO-Key` header
   - Use `/data/last` (single record), not `/data` (whole history)
5. **Get Dictionary Value** for key `value`
6. **Show Notification**: "PC is: [Dictionary Value]"

## Triggers

- Siri: name the Shortcut and say it.
- Lock Screen / Home Screen widget.
- Back Tap: Settings → Accessibility → Touch → Back Tap.

> The Shortcut stores your Adafruit IO key. If you share it, regenerate the key.
