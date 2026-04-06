from pyModbusTCP.client import ModbusClient
import time

# Insira o IP que o ESP32 imprimiu no log Serial (ex: "192.168.137.171")
ESP32_IP = "192.168.137.225"
PORT = 502

# Configura o cliente Modbus (O Notebook é o Mestre)
client = ModbusClient(host=ESP32_IP, port=PORT, auto_open=True, auto_close=True)

print(f"Iniciando Cliente Modbus TCP...\nTentando conectar ao Servidor no IP {ESP32_IP}:{PORT}")

while True:
    # 1. TESTE DE LEITURA (Lendo os dados gerados pelo ESP32)
    # Lendo 2 registradores a partir do endereço 0 (REG_IN_0 e REG_IN_1)
    regs_in = client.read_holding_registers(0, 2)

    if regs_in:
        print(f"Lido do ESP32 -> REG_IN_0: {regs_in[0]} | REG_IN_1: {regs_in[1]}")
    else:
        print("Falha ao ler os registradores 0 e 1 do ESP32.")

    # 2. TESTE DE ESCRITA (Enviando comandos para o ESP32 atuar)
    # Escrevendo os valores [50, 100] nos registradores a partir do endereço 2 (REG_OUT_0 e REG_OUT_1)
    valores_para_escrever = [50, 100]
    write_success = client.write_multiple_registers(2, valores_para_escrever)
    
    if write_success:
        print(f"Escrito no ESP32 -> REG_OUT_0: {valores_para_escrever[0]} | REG_OUT_1: {valores_para_escrever[1]}")
    else:
        print("Falha ao escrever nos registradores 2 e 3 do ESP32.")

    print("-" * 40)
    time.sleep(2)