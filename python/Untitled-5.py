# FILEPATH: 
# BEGIN: bks8u2gn22jw
import tkinter as tk
from tkinter import messagebox
import random

class TicTacToe:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title("井字棋游戏")
        self.window.resizable(0, 0)
        self.board = [[' ' for _ in range(3)] for _ in range(3)]
        self.current_player = 'X'
        self.x_score = 0
        self.o_score = 0
        self.buttons = []

        self.create_board()
        self.create_scoreboard()

    def create_board(self):
        for i in range(3):
            row_buttons = []
            for j in range(3):
                button = tk.Button(self.window, text=' ', font=('Arial', 20), width=5, height=2,
                                   command=lambda i=i, j=j: self.make_move(i, j))
                button.grid(row=i, column=j)
                row_buttons.append(button)
            self.buttons.append(row_buttons)

    def create_scoreboard(self):
        self.score_label = tk.Label(self.window, text=f"X: {self.x_score}  O: {self.o_score}", font=('Arial', 16))
        self.score_label.grid(row=3, columnspan=3)

    def make_move(self, i, j):
        if self.board[i][j] == ' ':
            self.board[i][j] = self.current_player
            self.buttons[i][j].config(text=self.current_player)
            if self.check_win(self.current_player):
                self.show_winner(self.current_player)
            elif self.is_board_full():
                self.show_draw()
            else:
                self.current_player = 'O' if self.current_player == 'X' else 'X'
                if self.current_player == 'O':
                    self.ai_move()

    def ai_move(self):
        empty_cells = [(i, j) for i in range(3) for j in range(3) if self.board[i][j] == ' ']
        if empty_cells:
            i, j = random.choice(empty_cells)
            self.make_move(i, j)

    def check_win(self, player):
        for row in self.board:
            if all(cell == player for cell in row):
                return True
        for col in range(3):
            if all(self.board[row][col] == player for row in range(3)):
                return True
        if all(self.board[i][i] == player for i in range(3)) or all(self.board[i][2 - i] == player for i in range(3)):
            return True
        return False

    def is_board_full(self):
        return all(cell in ['X', 'O'] for row in self.board for cell in row)

    def show_winner(self, player):
        if player == 'X':
            self.x_score += 1
        else:
            self.o_score += 1
        self.update_scoreboard()
        messagebox.showinfo("游戏结束", f"玩家 {player} 获胜！")
        self.reset_board()

    def show_draw(self):
        messagebox.showinfo("游戏结束", "这是一场平局！")
        self.reset_board()

    def update_scoreboard(self):
        self.score_label.config(text=f"X: {self.x_score}  O: {self.o_score}")

    def reset_board(self):
        for i in range(3):
            for j in range(3):
                self.board[i][j] = ' '
                self.buttons[i][j].config(text=' ')
        self.current_player = 'X'

    def run(self):
        self.window.mainloop()

if __name__ == "__main__":
    game = TicTacToe()
    game.run()
# END: bks8u2gn22jw
