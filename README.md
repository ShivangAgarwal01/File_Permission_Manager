# SecureFile: Windows GUI-Based File Permissions Manager with Encryption

## 📌 Overview

SecureFile is a Windows-based C++ application that allows users to easily view, grant, revoke, and automate file permissions using a graphical interface. Additionally, the project provides a built-in XOR-based encryption/decryption feature for securing sensitive files — all without opening the command line.

Developed using the Win32 API, the application is designed for simplicity and accessibility, especially for non-technical users managing local file security on Windows systems.

---

## 🚀 Features

- 🔐 Grant, Revoke, and View file permissions (Read, Write, Full, etc.)
- 🖼️ GUI-based interface using CreateWindow (no CMD needed)
- ⚙️ Executes icacls commands silently using CreateProcessA
- 📋 Real-time output display of permission status (captured via pipes)
- 🧾 Activity logging with timestamps in permissions_log.txt
- 🔏 Built-in XOR encryption & decryption (toggleable)
- 📂 Works with plain .txt and binary files

---

## 🛠️ Technologies Used

- Language: C++
- APIs: Win32 API
- Functions:
  - CreateWindow (for GUI)
  - CreateProcessA (for silent command execution)
  - ReadFile / WriteFile (pipe-based output)
- Tools: Visual Studio / g++, Windows OS

---

## 💡 How It Works

1. Enter file path (e.g. C:\Users\Me\Desktop\test.txt)
2. Choose an action:
   - View → shows current permissions
   - Grant → adds permission (e.g., Everyone:R)
   - Revoke → removes specified user permission
   - ReadOnly → revokes all but read access
   - Encrypt → obfuscates file content using XOR logic
   - Decrypt → restores original content

3. All actions are logged in permissions_log.txt
4. Output is shown in the GUI’s output label

---

## 🔐 Encryption Info

XOR Cipher: Simple symmetric encryption
- Key: "mykey" (default hardcoded)
- Encryption is done in-place
- Decrypting the file with the same key restores original data

🧪 Example:

Before encryption: Hello World  
After encryption: ÆÕ÷Ê¼...  
After decryption: Hello World ✅

---

## 📷 Screenshots

[Insert your GUI screenshots here once available]

---

## 📂 File Structure
