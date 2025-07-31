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
                self.ai_move()

def ai_move(self):
        empty_cells = [(i, j) for i in range(3) for j in range(3) if self.board[i][j] == ' ']
        if empty_cells:
            i, j = random.choice(empty_cells)
            self.make_move(i, j)