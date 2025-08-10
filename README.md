# Verificación en Dos Pasos para HMI Industrial

Este repositorio contiene los ejecutables y scripts necesarios para implementar un sistema de **verificación en dos pasos** en una HMI industrial.

---

## 📂 Estructura del repositorio

C:.
├───Ejecutables
│ └───SetUp
│ └───out
└───Scripts

- **Ejecutables/** → Contiene los archivos `.exe` requeridos para la instalación y ejecución del sistema.
- **Ejecutables/SetUp/** → Contiene el `exe` necesario para generar los archivos auxiliares para que funcione la herramienta.
- **Ejecutables/SetUp/out** → Contiene los archivos auxiliares generados por `mfa-setup.exe`.
- **Scripts/** → Contiene los scripts que deben ser integrados en el proyecto de la HMI.

---

## ⚙️ Instalación y configuración

### 1. **Generación de claves**

1. Ejecuta el programa `Ejecutables/SetUp/mfa-setup` en el equipo que ejecute la HMI. Esto generará el archivo `config.conf` con las llaves necesarias para que funcione la herramienta, también generará un archvo `clave_privada.pem` con la llave privada para la validación de usuarios por USB. Adicionalmente también generará un QR para impotar el inicio de sesión en [Authy](https://www.authy.com/) (en teoría es compatible con más clientes como Microsoft Autenticator, o similares, pero para las pruebas se ha utilziado Authy).
2. Copia el archivo `config.conf` en la carpeta `Ejecutables`, reemplazando el existente.
3. Importa el inicio de sesión en [Authy](https://www.authy.com/), es necesario crear una cuenta si no se dispone de una.
4. Copia el archivo `clave_privada.pem` en un USB, creando una carpeta `auth` dentro. Quedando una ruta final tal que así `E:/auth/clave_privada.pem`

### 2. **Ejecutables**

1. Copiar el contenido de la carpeta `Ejecutables` en el equipo que ejecute la HMI.
2. Asegurarse de copiar los ejecutables y el .conf en una ruta conocida.

### 3. **Scripts**

1. Abrir el proyecto de la HMI con el Premium HMI.
2. Copiar los scripts desde la carpeta `Scripts` del repositorio al proyecto de la HMI, recomendado carpte 001_HMI.
3. Modifica la ruta de trabajo de los scripts para que coincida con la ruta donde se han guardado los ejecutables, por ejemplo:  
   Ruta ejecutables: `C:\Script\Ejecutables`

```vba
' Cambiar la carpeta de trabajo a la del ejecutable
WshShell.CurrentDirectory = "C:\Script\Ejecutables"

---

## 🛠️ Configuración en la HMI

### Crear 2 variables en la HMI

- Nombre sugerido: `LogeadoXUsb`
- Tipo: **Bit**.
- Alcance: **Interna, no persistente**.

- Nombre sugerido: `ValidandoPswd`
- Tipo: **Bit** (dependiendo de la lógica de control).
- Alcance: **Interna, no persistente**.

### Crear un evento en la HMI

- **Evento de disparo**: Cada segundo (`_SysVar_:ActTimeSec`).
- **Acción**: Ejecutar el script LogXUsb.

### Configuración de Scripts de Log On

- Configurar los en el apartado usuarios, para los usuarios cuyo nivel de acceso sea 7, o superior, el script `ValidarPassword` como script de Log On, esto lanzará el script en cada incio de sesión de estos usuario con permisos elevados.

---

## 🚀 Uso

1. El usuario inicia el proceso en la HMI.
2. Se solicita la verificación adicional (segundo paso).
3. Si la verificación es correcta, el sistema permite la acción solicitada.
4. En caso contrario, se bloquea la operación.

---

## 📌 Notas

- Este sistema está diseñado para entornos industriales, asegúrese de cumplir con las normativas de seguridad aplicables.
- Los ejecutables y scripts deben mantenerse actualizados para garantizar la compatibilidad y la seguridad.
- Para soporte, contacte con el equipo de desarrollo o el administrador de la HMI.

---
