# 🎮 LED Pattern Game with FreeRTOS on ESP32

## 📌 Overview
This project utilizes **FreeRTOS on the ESP32** and integrates with the **Blynk app** to create an **interactive memory-based LED pattern game**. Players must **replicate an LED sequence** using **virtual buttons** in the app. The difficulty increases as the player progresses through levels, making it a **challenging and engaging game**.

---

## 🎯 Features
✔ **Multitasking with FreeRTOS** – Handles LED control, user input, and LCD updates.  
✔ **Blynk App Integration** – Players interact via virtual buttons.  
✔ **Adaptive Difficulty** – The sequence length increases with levels.  
✔ **Score & Mistake Tracking** – Keeps track of progress and errors.  
✔ **Dynamic Feedback** – LCD displays game prompts and status.  

---

## 🛠 Components & Requirements
### 🔌 **Hardware Components**
- **ESP32**
- **Breadboard**
- **4 Different-Colored LEDs**
- **Resistors (220Ω**
- **LCD Display**
- **Jumper Wires**

### 📱 **Blynk App Features**
- **Start Game Button** – Resets game variables (score, level, mistakes).  
- **Four LED Buttons** – Used to replicate the LED sequences.  

---

## 🎮 Game Flow

1️⃣ **Game Initialization**  
- The player **presses the start button** on the Blynk app.  
- All game variables (**score, level, mistakes**) are reset.  

2️⃣ **Pattern Generation**  
- A **random LED sequence** is displayed.  
- The sequence length **increases as the player progresses**.  

| **Level** | **Number of LEDs in Sequence** |
|----------|------------------------------|
| Level 1  | 3 LEDs |
| Level 2  | 4 LEDs |
| Level 3  | 5 LEDs |
| ...      | ... |
| Level 20 | 22 LEDs |

3️⃣ **Player Interaction**  
- The **LEDs flash in sequence** while the LCD displays "Watch closely!"  
- The player **replicates the sequence** using virtual buttons in the Blynk app.  

4️⃣ **Score & Mistake Handling**  
- **Successful Input** → Player advances to the next level with a longer sequence.  
- **Incorrect Input** → Mistake counter increases.  
- **Max 3 Mistakes Per Level** → If exceeded, game over.  

5️⃣ **Game Completion**  
- If the player **successfully completes all 20 levels**, they win! 🎉  
- The player can **choose to restart the game** from Level 1.  

---

## 🔄 Example Game Sequence
1. **Player Presses Start**
2. **LED Pattern is Displayed**:
   - **LED 1 ON → OFF, LED 1 ON → OFF, LED 3 ON → OFF**
3. **Player Replicates the Sequence** in the Blynk App.
4. **Successful Entry → Next Level**:
   - **LED 1 ON → OFF, LED 3 ON → OFF, LED 3 ON → OFF, LED 1 ON → OFF**
5. **Game Continues Until Level 20 or Max Mistakes Reached**.

---


## 📎 How to Run the Project?

### 1️⃣ Install Required Software & Libraries
- Install **Arduino IDE** & **ESP32 Board Support**.
- Install the **Blynk Library** (`BlynkSimpleEsp32.h`).
- Install **FreeRTOS Library** for multitasking.

---

### 2️⃣ Upload the Code to ESP32
- Open **Lab_Project.ino** in **Arduino IDE**.
- Connect the **ESP32** via **USB**.
- Select the correct **ESP32 Board & Port**.
- Click **Upload**.

---

### 3️⃣ Configure the Blynk App
- Create a **new project** in the **Blynk app**.
- Add **four virtual buttons (V1-V4)** for LED input.
- Add a **start button (V5)** to initiate the game.
- Obtain your **Blynk Authentication Token** and add it to the code.

---

### 4️⃣ Play the Game!  
1. **Press Start** in the Blynk app.  
2. **Watch the LED sequence** carefully.  
3. **Replicate the sequence** using the buttons.  
4. Progress through levels and **try to reach Level 20!** 🎯  

---

## 🔍 Expected Output
- **LEDs flash in a sequence** as a pattern to remember.  
- **LCD displays prompts** like **"Watch closely!"** or **"Repeat the sequence!"**.  
- If the sequence is **correct**, the **level increases**.  
- If mistakes exceed **3 per level**, **game over** is displayed.  

---

## 📢 Conclusion
This project demonstrates:  
✔ **Real-time multitasking** using **FreeRTOS on ESP32**.  
✔ **Blynk-based IoT game control** with **LED interaction**.  
✔ **Adaptive difficulty** for a challenging experience.  
✔ **Engaging game mechanics** with **memory-based gameplay**.  

---

## 👨‍💻 Authors
This project was developed by:

- **Yazeed Hamdan**  
- **Mahmoud Hamdan**
- **Anas Hawamdah**    

Feel free to reach out for collaboration or inquiries! 😊  

---

## 📫 Contact
For any questions or discussions, feel free to reach out:

[![Email](https://img.shields.io/badge/Email-D14836?style=for-the-badge&logo=gmail&logoColor=white)](mailto:yazedyazedl2020@gmail.com)  
[![LinkedIn](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin)](https://www.linkedin.com/in/yazeed-hamdan-59b83b281/)  

