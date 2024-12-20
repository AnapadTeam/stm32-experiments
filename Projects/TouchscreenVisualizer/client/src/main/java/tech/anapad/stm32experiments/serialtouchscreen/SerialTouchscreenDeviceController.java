package tech.anapad.stm32experiments.serialtouchscreen;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortEvent;
import com.fazecast.jSerialComm.SerialPortInvalidPortException;
import com.fazecast.jSerialComm.SerialPortMessageListener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenConfig;
import tech.anapad.stm32experiments.serialtouchscreen.model.TouchscreenTouch;

import java.util.Arrays;
import java.util.List;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * {@link SerialTouchscreenDeviceController} controls serial devices.
 */
public class SerialTouchscreenDeviceController {

    private static final Logger LOGGER = LoggerFactory.getLogger(SerialTouchscreenDeviceController.class);

    private SerialPort connectedSerialPort;

    public void stop() {
        if (connectedSerialPort != null) {
            connectedSerialPort.closePort();
        }
    }

    /**
     * Gets the available system {@link SerialPort} names.
     *
     * @return a {@link List} of {@link String}s
     */
    public List<String> getAvailableSerialPorts() {
        return Arrays.stream(SerialPort.getCommPorts())
                .map(SerialPort::getSystemPortName)
                .collect(Collectors.toList());
    }

    /**
     * Initializes the touchscreen serial port asynchronous data processing.
     *
     * @param serialSystemPortName       the serial system port name
     * @param baudRate                   the baud rate
     * @param touchscreenConfigConsumer  the {@link TouchscreenConfig} {@link Consumer}
     * @param touchscreenTouchesConsumer the {@link TouchscreenTouch[]} {@link Consumer}
     *
     * @throws SerialPortInvalidPortException the serial port invalid port exception
     */
    public void initTouchscreenSerialPort(String serialSystemPortName, int baudRate,
            Consumer<TouchscreenConfig> touchscreenConfigConsumer,
            Consumer<TouchscreenTouch[]> touchscreenTouchesConsumer) throws SerialPortInvalidPortException {
        connectedSerialPort = SerialPort.getCommPort(serialSystemPortName);
        connectedSerialPort.setBaudRate(baudRate);
        connectedSerialPort.addDataListener(new TouchscreenSerialPortMessageHandler(touchscreenConfigConsumer,
                touchscreenTouchesConsumer));
        connectedSerialPort.openPort();
    }

    /**
     * {@link TouchscreenSerialPortMessageHandler} handles touchscreen serial port data.
     */
    private static class TouchscreenSerialPortMessageHandler implements SerialPortMessageListener {

        private final Consumer<TouchscreenConfig> touchscreenConfigConsumer;
        private final Consumer<TouchscreenTouch[]> touchscreenTouchesConsumer;
        private final TouchscreenTouch[] touchscreenTouches;

        /**
         * Instantiates a new {@link TouchscreenSerialPortMessageHandler}.
         *
         * @param touchscreenConfigConsumer  the {@link TouchscreenConfig} {@link Consumer}
         * @param touchscreenTouchesConsumer the {@link TouchscreenTouch[]} {@link Consumer}
         */
        public TouchscreenSerialPortMessageHandler(Consumer<TouchscreenConfig> touchscreenConfigConsumer,
                Consumer<TouchscreenTouch[]> touchscreenTouchesConsumer) {
            this.touchscreenConfigConsumer = touchscreenConfigConsumer;
            this.touchscreenTouchesConsumer = touchscreenTouchesConsumer;
            touchscreenTouches = new TouchscreenTouch[10];
        }

        @Override
        public byte[] getMessageDelimiter() {
            return new byte[]{'\r', '\n'}; // CRLF
        }

        @Override
        public boolean delimiterIndicatesEndOfMessage() {
            return true;
        }

        @Override
        public int getListeningEvents() {
            return SerialPort.LISTENING_EVENT_DATA_RECEIVED;
        }

        @Override
        public void serialEvent(SerialPortEvent serialPortEvent) {
            String serialMessage = new String(serialPortEvent.getReceivedData());
            LOGGER.debug("Received serial message: {}", serialMessage);

            if (serialMessage.startsWith("c")) {
                // Config data format:
                // c,<xResolution>,<yResolution>

                String[] serialMessageCSV = serialMessage.trim().split(",");

                if (serialMessageCSV.length != 3 || !serialMessageCSV[0].equals("c")) {
                    LOGGER.warn("Received unknown message format: {}", serialMessage);
                    return;
                }

                int xResolution;
                int yResolution;
                try {
                    xResolution = Integer.parseInt(serialMessageCSV[1]);
                    yResolution = Integer.parseInt(serialMessageCSV[2]);
                } catch (Exception exception) {
                    LOGGER.error("Invalid X/Y resolution numbers from config message: {}", serialMessage, exception);
                    return;
                }

                LOGGER.info("X/Y resolution numbers from config message: {}x{}", xResolution, yResolution);
                TouchscreenConfig touchscreenConfig = new TouchscreenConfig(xResolution, yResolution);
                touchscreenConfigConsumer.accept(touchscreenConfig);
            } else if (serialMessage.startsWith("t")) {
                // Touch data format:
                // t,<index>,<id>,<x>,<y>,<size>,<index>,<id>,<x>,<y>,<size>,<...>

                final String[] serialMessageCSV = serialMessage.trim().split(",");
                final int touchDataContentLength = 5;

                // Message must have content length that is a multiple of 5
                if (serialMessageCSV.length < touchDataContentLength + 1 ||
                        ((serialMessageCSV.length - 1) % touchDataContentLength) != 0) {
                    LOGGER.warn("Received unknown message format: {}", serialMessage);
                    return;
                }

                for (int index = 1; index < serialMessageCSV.length; index += touchDataContentLength) {
                    int touchIndex;
                    int id;
                    int x;
                    int y;
                    int size;
                    try {
                        touchIndex = Integer.parseInt(serialMessageCSV[index]);
                        id = Integer.parseInt(serialMessageCSV[index + 1]);
                        x = Integer.parseInt(serialMessageCSV[index + 2]);
                        y = Integer.parseInt(serialMessageCSV[index + 3]);
                        size = Integer.parseInt(serialMessageCSV[index + 4]);
                    } catch (Exception exception) {
                        LOGGER.error("Invalid touch data from message: {}", serialMessage);
                        return;
                    }

                    if (touchIndex >= touchscreenTouches.length) {
                        LOGGER.error("Touch index from message is out of bounds: {}", serialMessage);
                        return;
                    }

                    TouchscreenTouch touchscreenTouch = null;
                    if (x != 0 && y != 0 && size != 0) {
                        touchscreenTouch = new TouchscreenTouch(id, x, y, size);
                        LOGGER.info("{} at index {}", touchscreenTouch, touchIndex);
                    }

                    touchscreenTouches[touchIndex] = touchscreenTouch;
                }

                touchscreenTouchesConsumer.accept(touchscreenTouches);
            } else {
                LOGGER.warn("Received unknown message format: {}", serialMessage);
                return;
            }
        }
    }
}
