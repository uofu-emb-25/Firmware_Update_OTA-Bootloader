from flask import Flask, request, redirect, url_for, render_template_string
from werkzeug.utils import secure_filename
import os
import struct
import serial
from time import sleep

# —— Configuration —— #
UPLOAD_FOLDER = 'uploads'
ALLOWED_EXTENSIONS = {'bin'}
SERIAL_PORT = '/dev/tty.usbserial-A5069RR4'
BAUDRATE    = 115200
WINDOW_SIZE = 128

# —— Flask setup —— #
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# —— Serial setup —— #
serial_com = serial.Serial(port=SERIAL_PORT, baudrate=BAUDRATE, timeout=15)

# —— CRC & send logic —— #
def compute_crc(buff, length):
    crc = 0xFFFFFFFF
    for byte in buff[:length]:
        crc ^= byte
        for _ in range(32):
            if crc & 0x80000000:
                crc = (crc << 1) ^ 0x04C11DB7
            else:
                crc <<= 1
    return crc & 0xFFFFFFFF

def send_data_serial(data):
    packet = data + struct.pack('<I', compute_crc(data, len(data)))
    serial_com.write(packet)
    sleep(0.2)   # small inter‐packet delay

def update_flash_mem(filepath):
    size       = os.path.getsize(filepath)
    size_copy  = size
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(WINDOW_SIZE)
            if not chunk:
                break
            send_data_serial(chunk)
            size -= len(chunk)
            percent = int(100*(size_copy - size)/size_copy)
            bar     = '#' * int(50*(size_copy - size)/size_copy)
            print(f'\rUpdating flash: {percent:3d}% |{bar:<50}|', end='')
    print('\nAll data sent, sending finalize command…')
    message = 0x42
    message_bytes = bytes([message])
    send_data_serial(message_bytes)


# —— Helpers —— #
def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

# —— Routes —— #
HTML_FORM = """
<!doctype html>
<title>Firmware Uploader</title>
<h1>Upload .bin Firmware</h1>
<form method=post enctype=multipart/form-data>
  <input type=file name=file accept=".bin">
  <input type=submit value=Upload>
</form>
"""

@app.route('/', methods=['GET', 'POST'])
def upload():
    if request.method == 'POST':
        file = request.files.get('file')
        if not file or file.filename == '':
            return 'No file selected', 400
        if not allowed_file(file.filename):
            return 'Only .bin files are allowed', 400

        filename = secure_filename(file.filename)
        path     = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(path)

        # fire off the flashing routine
        try:
            update_flash_mem(path)
        except Exception as e:
            return f'Error during flash update: {e}', 500

        return '✅ Firmware update complete!'
    return render_template_string(HTML_FORM)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)