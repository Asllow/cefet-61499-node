from pyModbusTCP.client import ModbusClient
import time

ESP32_IP = "192.168.137.76" # Mude para o seu IP atual
PORT = 502

def to_signed(val_16bit):
    """ Converte o uint16 recebido pelo Modbus de volta para inteiro com sinal """
    return val_16bit if val_16bit < 32768 else val_16bit - 65536

client = ModbusClient(host=ESP32_IP, port=PORT, auto_open=True, auto_close=True)
print(f"Iniciando Cliente Modbus TCP...\nTentando conectar ao Servidor no IP {ESP32_IP}:{PORT}")

while True:
    regs_in = client.read_holding_registers(0, 2)

    if regs_in:
        # Aplica o descodificador de Complemento de 2
        erro_calculado = to_signed(regs_in[0])
        leitura_taco = to_signed(regs_in[1])
        print(f"Lido do ESP32 -> Erro Calculado (REG_0): {erro_calculado} | Taco (REG_1): {leitura_taco}")
    else:
        print("Falha ao ler os registradores do ESP32.")

    # Escrevendo o Setpoint no registrador 4 (Primeiro registrador de saída configurado)
    setpoint = 50
    write_success = client.write_multiple_registers(4, [setpoint])
    
    if write_success:
        print(f"Escrito no ESP32 -> Setpoint (REG_4): {setpoint}")

    print("-" * 40)
    time.sleep(2)