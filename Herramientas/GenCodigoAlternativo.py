from datetime import datetime
import os

def codigo_maestro(pin: int, fecha: str = None) -> str:
    """
    Genera un código maestro de 6 dígitos
    basado en un PIN y la fecha (formato AAAAMMDD).
    
    :param pin: PIN numérico (ej: 1111)
    :param fecha: Fecha en formato 'yyyymmdd'. 
                  Si no se pasa, usa la fecha actual.
    :return: string de 6 dígitos
    """
    if fecha is None:
        fecha = datetime.now().strftime("%Y%m%d")
    
    raw = int(pin) * int(fecha)
    code = raw % 1_000_000  # últimos 6 dígitos
    return f"{code:06d}"    # siempre 6 dígitos con ceros a la izquierda


if __name__ == "__main__":

    print("Generador de código maestro")
    input_user = input("Introduce tu PIN (código de ingeniero, últimos 4 dígitos del IE de trabajo 250001234 -> 1234) \n > ")
    if input_user.isdigit() and len(input_user) == 4:
        pin = int(input_user)   
    else:
        print("PIN inválido. Debe ser un número de 4 dígitos.")
        exit(1)

    input_user = input("Introduce la fecha de la HMI en el siguiente formato YYYYMMDD, dejar en blanco para utilizar la fecha de hoy \n > ")

    if input_user.isdigit() and len(input_user) == 8:
        fecha = int(input_user)   
    else:
        fecha = datetime.now().strftime("%Y%m%d")  # fecha de hoy
    

    os.system('cls' if os.name == 'nt' else 'clear')  # Limpiar la consola

    print("PIN:", pin)
    print("Fecha:", fecha)
    print("Código maestro:", codigo_maestro(pin, fecha))