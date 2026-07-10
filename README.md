# 🤖 AI Chatbot — Qt Based Desktop Application

**AI Chatbot** is a modern, desktop-based conversational AI application built using **Qt (C++)** and powered by the **Groq API**. It offers a sleek dark-themed interface with secure authentication, real-time chat, voice input, file/image sharing, and persistent chat history — bringing a full-featured AI assistant experience right to your desktop.

---

## ✨ Features

- 🔐 **Secure Login & Signup System** — User authentication with email, username, and password
- 💬 **Smart AI Responses** — Real-time conversational AI powered by the **Groq API**
- 🎙️ **Voice Input Support** — Speak your messages instead of typing
- 🖼️ **Image & File Upload** — Attach images and files directly in the chat
- 😊 **Emoji Support** — Express yourself with an integrated emoji picker
- 🕘 **Chat History Saved** — Every conversation is saved as a session and can be resumed anytime
- ➕ **New Chat / Delete Chat** — Easily manage multiple conversations
- 🟢 **Online Status Indicator** — See your connection status at a glance
- 🎨 **Modern Dark UI** — Clean, minimal, and easy on the eyes

---

## 🖥️ Screenshots

| Sign In | Sign Up |
|---------|---------|
| Secure login with username & password | Create an account with email, username & password |

| Chat Interface | Chat History |
|----------------|---------------|
| Clean chat window with voice, image, and file support | Browse and continue previous conversations |

*(Add your actual screenshot files to a `/screenshots` folder and update the paths above, e.g. `![Sign In](screenshots/signin.png)`)*

---

## 🛠️ Tech Stack

- **Framework:** Qt (C++)
- **AI Backend:** [Groq API](https://groq.com/)
- **UI:** Custom Qt Widgets / QML with dark theme styling

---

## 🚀 Getting Started

### Option 1: Run the App Directly (No Qt Installation Required)

If you just want to **try the app** without setting up a development environment:

1. Go to the [`release`](./release) folder in this repository.
2. Download the entire folder (it contains the `.exe` file along with all required Qt DLLs).
3. Run the `.exe` file — **no need to install Qt or any dependencies.**

> ⚠️ Make sure to keep all files in the `release` folder together in the same directory. The `.exe` depends on the accompanying DLL files to run correctly.

### Option 2: Build from Source (For Developers)

If you want to explore, modify, or build the project yourself:

#### Prerequisites
- [Qt Framework](https://www.qt.io/download) (Qt Creator recommended)
- A C++ compiler (MinGW / MSVC)
- A [Groq API key](https://console.groq.com/) (free to generate)

#### Steps
```bash
# Clone the repository
git clone https://github.com/<your-username>/QT-based-Ai-Chatbot.git

# Open the project
cd QT-based-Ai-Chatbot
```

1. Open the `.pro` file in **Qt Creator**.
2. Add your **Groq API key** in the designated config file / source file (see `config.h` or `.env`, depending on your setup).
3. Build and run the project (`Ctrl + R` in Qt Creator).

---

## 🔑 API Key Setup

This project uses the **Groq API** for AI responses. To run it locally:

1. Sign up at [console.groq.com](https://console.groq.com/) and generate an API key.
2. Add the key to the appropriate configuration location in the source code.
3. **Never commit your API key to GitHub** — consider using a `.env` file or a config file excluded via `.gitignore`.

---

## 📁 Project Structure

```
QT-based-Ai-Chatbot/
├── release/          # Prebuilt executable + required Qt DLLs (ready to run)
├── screenshots/       # UI screenshots used in this README
├── *.cpp              # Source code files
├── *.h                # Header files
├── *.ui               # UI form files
├── *.qrc              # Resource file (icons, images, emoji assets)
├── AI-Chatbot.pro      # Qt project file
└── README.md
```

*(All source, header, and UI files are kept flat in the project root alongside the `.pro` file — this matches how Qt Creator generates and expects the project layout by default.)*

---

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to open an issue or submit a pull request.

---

## 📄 License

This project is open source. Add your preferred license (e.g., MIT) here.

---

## 👤 Author

Developed by **Awais Raza** — feel free to connect and share feedback!
