#Code của khối nhận dạng
import cv2
from ultralytics import YOLO
import paho.mqtt.client as mqtt


model = YOLO('runs/detect/train11/weights/best.pt')

# Mở camera và đặt độ phân giải 1280x720
cap = cv2.VideoCapture(1)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

# Kiểm tra camera
if not cap.isOpened():
    print("Không thể mở camera")
    exit()

# Cấu hình MQTT
MQTT_USER = "phi_den"  # Thêm tên người dùng
MQTT_PASSWORD = "123Phi"
MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883
MQTT_TOPIC_LABEL = "Label"
MQTT_TOPIC_MOTOR = "Motor"
MQTT_TOPIC_SERVO = "Servo"
MQTT_TOPIC_COUNT = "Count"
TOPIC_State = "State"

# Biến lưu trạng thái
motor_state = "Unknown"
servo_state = "Unknown"
light_state = "Off"
count = 0
exit_program = False
flat = 1
detect = ""
State = ""

# Hàm callback khi nhận dữ liệu từ MQTT
def on_message(client, userdata, msg):
    global motor_state, servo_state, count, State
    topic = msg.topic
    payload = msg.payload.decode()

    if topic == MQTT_TOPIC_MOTOR:
        motor_state = "On" if payload == "On" else "Off"
    elif topic == MQTT_TOPIC_SERVO:
        servo_state = "Active" if payload == "Active" else "Inactive"
    elif topic == MQTT_TOPIC_COUNT:
        count = payload
    elif topic == TOPIC_State:
        State = payload
# Kết nối MQTT
def on_connect(client, userdata, flags, rc):
    print(f"Kết nối đến MQTT Broker với mã trạng thái {rc}")
    client.subscribe(MQTT_TOPIC_MOTOR)
    client.subscribe(MQTT_TOPIC_SERVO)
    client.subscribe(MQTT_TOPIC_COUNT)
    client.subscribe(TOPIC_State)
client = mqtt.Client()
client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_start()

# Hàm xử lý sự kiện click chuột
def on_mouse(event, x, y, flags, param):
    global light_state, exit_program
    # Vùng click cho nút đèn
    if event == cv2.EVENT_LBUTTONDOWN:
        if 1010 <= x <= 1100 and 740 <= y <= 790:
            # Cập nhật biến exit_program để thoát chương trình
            exit_program = True
            print("Exit button clicked, exiting program...")

# Đăng ký callback chuột cho cửa sổ
cv2.namedWindow('Sprouted Potatoes Detection', cv2.WINDOW_NORMAL)
cv2.resizeWindow('Sprouted Potatoes Detection', 1280, 720)
cv2.setMouseCallback('Sprouted Potatoes Detection', on_mouse)

# Hàm vẽ văn bản
def draw_text(frame, text, position, font_scale=0.85, font_thickness=1, text_color=(0, 0, 0)):
    x, y = position
    font = cv2.FONT_HERSHEY_COMPLEX
    # Vẽ văn bản chính (màu đen)
    cv2.putText(frame, text, (x, y), font, font_scale, text_color, font_thickness, lineType=cv2.LINE_AA)

while True:
    # Đọc từng frame từ camera
    ret, frame = cap.read()
    if not ret:
        print("Không thể nhận frame từ camera")
        break

    # Thêm viền cho khung camera (10px với màu đỏ)
    frame_with_border = cv2.copyMakeBorder(frame, 2, 2, 2, 2, cv2.BORDER_CONSTANT, value=(0, 0, 255))

    # Tạo một khung trắng để chứa các nút phía dưới camera
    button_area = 100  # Chiều cao khu vực chứa nút
    full_frame = cv2.copyMakeBorder(frame_with_border, 0, button_area, 0, 0, cv2.BORDER_CONSTANT, value=(0, 0, 0))

    # Vẽ các nút trạng thái trên khung hình (tại vùng bên dưới khung hình camera)
    # Nút trạng thái Motor
    cv2.rectangle(full_frame, (100, 740), (350, 790), (0, 0, 255) if motor_state == "Off" else (0, 255, 0), -1)
    draw_text(full_frame, f"Motor: {motor_state}", (110, 770))

    # Nút trạng thái Servo
    cv2.rectangle(full_frame, (360, 740), (610, 790), (0, 0, 255) if servo_state == "Inactive"else (0, 255, 0) , -1)
    draw_text(full_frame, f"Servo: {servo_state}", (370, 770))

    # Nút hiển thị trạng thái Detect
    cv2.rectangle(full_frame, (620, 740), (890, 790), (0, 0, 255) if detect == "Yes" else (0, 255, 0), -1)
    draw_text(full_frame, f"Detect: {detect}", (630, 770))

    # Nút hiển thị số lượng phát hiện (counter)
    cv2.rectangle(full_frame, (900, 740), (1000, 790), (255, 255, 255), -1)  # Vẽ nút cho counter
    draw_text(full_frame, f"{count}", (910, 770))  # Hiển thị giá trị counter

    # Nút thoát chương trình
    cv2.rectangle(full_frame, (1010, 740), (1100, 790), (0, 0, 255), -1)
    draw_text(full_frame, "Exit", (1025, 770))
    #kiểm tra trạng thái
    if State == "On":
        results = model(frame_with_border)
        # Vẽ khung phát hiện của YOLOv8 lên trên khung camera
        for result in results:
            for box in result.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                label = result.names[box.cls[0].item()]
                confidence = box.conf[0].item()

                # Vẽ khung chữ nhật quanh vật thể
                cv2.rectangle(full_frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                text = f"{label} ({confidence:.2f})"
                draw_text(full_frame, text, (x1, y1 - 10), 0.89, 1, text_color=(0, 0, 255))
                # So sánh với nhãn và confidence trước đó
                if label == "mam":
                    detect = "Yes"
                    client.publish(MQTT_TOPIC_LABEL, detect)
                    State = "Off"
                    break
                else:
                    detect = "No"
                    client.publish(MQTT_TOPIC_LABEL, detect)
                    State = "Off"
                    break
    # Hiển thị khung hình đã chú thích (annotated frame) với các nút
    cv2.imshow('Sprouted Potatoes Detection', full_frame)

    # Thoát nếu nhấn nút thoát hoặc phím 'q'
    if exit_program or cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Giải phóng camera và đóng tất cả cửa sổ
cap.release()
cv2.destroyAllWindows()


