import socket

# Configura a porta e manda escutar em todas as placas de rede do PC (0.0.0.0)
UDP_IP = "0.0.0.0"
UDP_PORT = 5000

# Cria o socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Servidor UDP rodando... Escutando na porta {UDP_PORT}")
print("Aguardando datagramas do ESP32...\n")

while True:
    # Aguarda receber os dados
    data, addr = sock.recvfrom(1024) # buffer de 1024 bytes
    print(f"[{addr[0]}:{addr[1]}] -> Tacogerador: {data.decode('utf-8')}")