import tkinter as tk

window = tk.Tk()

window.geometry("600x400")

events = []

# Create an event handler
def handle_btn_press():
    print("You pressed the button!")

btn = tk.Button( text="Click me!",
    width=25,
    height=5,
    bg="black",
    fg="white",
    command=handle_btn_press
)

btn.pack()

window.mainloop()