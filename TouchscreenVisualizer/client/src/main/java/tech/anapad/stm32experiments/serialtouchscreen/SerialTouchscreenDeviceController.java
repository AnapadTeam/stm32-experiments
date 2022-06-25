package tech.anapad.stm32experiments.serialtouchscreen;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortEvent;
import com.fazecast.jSerialComm.SerialPortInvalidPortException;
import com.fazecast.jSerialComm.SerialPortMessageListener;
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
            // TODO
        }
    }
}
