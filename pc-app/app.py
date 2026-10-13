import customtkinter as ctk
import serial
import threading
import time

# --- Cấu hình cổng Arduino ---
PORT = 'COM3' 
BAUDRATE = 9600

# Khai báo từ điển ánh xạ
TRASH_TYPES = {
    "0": "Mặc định (Hãy bỏ rác)",
    "1": "Rác hữu cơ",
    "2": "Rác vô cơ",
    "3": "Rác tái chế",
    "4": "Pin và Ắc quy"
}

try:
    arduino = serial.Serial(PORT, BAUDRATE, timeout=1)
    time.sleep(2)
    print(f"Đã kết nối thành công với Arduino tại cổng {PORT}")
except Exception as e:
    arduino = None
    print(f"Lỗi: Không thể kết nối với Arduino. Vui lòng kiểm tra cáp cắm.")

# --- Thiết lập giao diện ---
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

app = ctk.CTk()
app.title("Hệ Thống Smart Trash STEM")
app.geometry("550x650") # Nới form rộng ra chút để chứa 4 nút

# --- Các hàm chức năng ---
def open_lid_manual():
    if arduino:
        arduino.write(b'O')
        status_label.configure(text="Trạng thái thao tác: Đã gửi lệnh MỞ nắp")

def close_lid_manual():
    if arduino:
        arduino.write(b'C')
        status_label.configure(text="Trạng thái thao tác: Đã gửi lệnh ĐÓNG nắp")

def set_auto_mode():
    if arduino:
        arduino.write(b'A') 
        status_label.configure(text="Trạng thái thao tác: Đã gửi lệnh BẬT TỰ ĐỘNG")

def set_manual_mode():
    if arduino:
        arduino.write(b'M') # Gửi lệnh 'M' xuống Arduino
        status_label.configure(text="Trạng thái thao tác: Đã gửi lệnh BẬT THỦ CÔNG")

def change_trash_type(choice):
    if arduino:
        for key, value in TRASH_TYPES.items():
            if value == choice:
                arduino.write(key.encode())
                status_label.configure(text=f"Trạng thái thao tác: Đã chuyển sang {value}")
                break

def read_data_from_arduino():
    while True:
        if arduino and arduino.in_waiting > 0:
            try:
                data = arduino.readline().decode('utf-8', errors='ignore').strip()
                
                # Cập nhật Dung lượng rác
                if "Volume:" in data:
                    distance_str = data.split("Volume:")[1].strip()
                    distance = int(distance_str)

                    if distance < 0:
                        # -1 = cảm biến không nhận được tiếng vọng, giữ nguyên hiển thị cũ
                        pass
                    elif distance < 5:
                        trash_status.configure(text="THÙNG RÁC ĐÃ ĐẦY!", text_color="#ff4c4c")
                        trash_progress.set(1.0)
                        trash_progress.configure(progress_color="#ff4c4c")
                    elif distance < 15:
                        trash_status.configure(text="Đang có rác (Mức Vừa)", text_color="#ffcc00")
                        trash_progress.set(0.5)
                        trash_progress.configure(progress_color="#ffcc00")
                    else:
                        trash_status.configure(text="Thùng rỗng", text_color="#4caf50")
                        trash_progress.set(0.1)
                        trash_progress.configure(progress_color="#4caf50")
                        
                # Cập nhật Loại Rác
                elif "TYPE:" in data:
                    type_idx = data.split("TYPE:")[1].strip()
                    if type_idx in TRASH_TYPES:
                        type_var.set(TRASH_TYPES[type_idx])
                        
                # Bắt trạng thái Chế độ (Auto/Manual)
                elif "SYSTEM_MODE: MANUAL" in data:
                    system_mode_label.configure(text="Chế độ: THỦ CÔNG (Tắt cảm biến)", text_color="#ffcc00")
                elif "SYSTEM_MODE: AUTO" in data:
                    system_mode_label.configure(text="Chế độ: TỰ ĐỘNG (Bật cảm biến)", text_color="#00bfff")
                        
                # Cập nhật trạng thái nắp
                elif "Lid opened" in data:
                    hardware_status.configure(text="Hệ thống: Nắp đang mở", text_color="#00bfff")
                elif "Lid closed" in data:
                    hardware_status.configure(text="Hệ thống: Nắp đã đóng", text_color="gray")
                    
            except Exception as e:
                pass

# --- Xây dựng bố cục UI ---
title = ctk.CTkLabel(app, text="BẢNG ĐIỀU KHIỂN TRUNG TÂM", font=("Arial", 24, "bold"))
title.pack(pady=(20, 10))

status_frame = ctk.CTkFrame(app)
status_frame.pack(pady=10, padx=20, fill="x")

system_mode_label = ctk.CTkLabel(status_frame, text="Chế độ: TỰ ĐỘNG (Bật cảm biến)", font=("Arial", 16, "bold"), text_color="#00bfff")
system_mode_label.pack(pady=(15, 5))

ctk.CTkLabel(status_frame, text="MỨC ĐỘ CHỨA", font=("Arial", 12, "bold"), text_color="gray").pack(pady=(10, 0))
trash_progress = ctk.CTkProgressBar(status_frame, width=400, height=25, corner_radius=10)
trash_progress.pack(pady=10)
trash_progress.set(0)
trash_status = ctk.CTkLabel(status_frame, text="Đang chờ dữ liệu...", font=("Arial", 20, "bold"))
trash_status.pack(pady=(0, 10))
hardware_status = ctk.CTkLabel(status_frame, text="Hệ thống: Khởi động...", font=("Arial", 14))
hardware_status.pack(pady=(0, 10))

control_frame = ctk.CTkFrame(app)
control_frame.pack(pady=10, padx=20, fill="x")

ctk.CTkLabel(control_frame, text="PHÂN LOẠI RÁC", font=("Arial", 12, "bold"), text_color="gray").pack(pady=(10, 5))
type_var = ctk.StringVar(value="Đang đồng bộ...")
type_dropdown = ctk.CTkOptionMenu(
    control_frame, 
    values=list(TRASH_TYPES.values()), 
    variable=type_var, 
    command=change_trash_type,
    width=250, height=35,
    font=("Arial", 14)
)
type_dropdown.pack(pady=(0, 10))

# --- BỐ CỤC NÚT BẤM DẠNG LƯỚI (GRID) ---
btn_frame = ctk.CTkFrame(control_frame, fg_color="transparent")
btn_frame.pack(fill="x", pady=10, padx=20)

# Chia tỉ lệ cột bằng nhau
btn_frame.columnconfigure(0, weight=1)
btn_frame.columnconfigure(1, weight=1)

# Dòng 1: Đổi Chế Độ
btn_auto = ctk.CTkButton(btn_frame, text="Bật Tự Động", command=set_auto_mode, fg_color="#17a2b8", hover_color="#138496", height=45, font=("Arial", 14, "bold"))
btn_auto.grid(row=0, column=0, padx=10, pady=10, sticky="ew")

btn_manual = ctk.CTkButton(btn_frame, text="Bật Thủ Công", command=set_manual_mode, fg_color="#fd7e14", hover_color="#e8590c", height=45, font=("Arial", 14, "bold"))
btn_manual.grid(row=0, column=1, padx=10, pady=10, sticky="ew")

# Dòng 2: Điều khiển nắp
btn_open = ctk.CTkButton(btn_frame, text="Mở Nắp", command=open_lid_manual, fg_color="#28a745", hover_color="#218838", height=45, font=("Arial", 14, "bold"))
btn_open.grid(row=1, column=0, padx=10, pady=10, sticky="ew")

btn_close = ctk.CTkButton(btn_frame, text="Đóng Nắp", command=close_lid_manual, fg_color="#dc3545", hover_color="#c82333", height=45, font=("Arial", 14, "bold"))
btn_close.grid(row=1, column=1, padx=10, pady=10, sticky="ew")

status_label = ctk.CTkLabel(app, text="Trạng thái thao tác: Sẵn sàng", font=("Arial", 12), text_color="gray")
status_label.pack(side="bottom", pady=15)

if arduino:
    thread = threading.Thread(target=read_data_from_arduino, daemon=True)
    thread.start()

app.mainloop()