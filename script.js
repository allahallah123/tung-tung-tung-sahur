const quizData = [
  {
    question:
      "JavaScript: Kết quả của `typeof null` là gì?",
    options: [
      "A. \"null\"",
      "B. \"object\"",
      "C. \"undefined\"",
      "D. \"number\"",
    ],
    correctIndex: 1,
  },
  {
    question:
      "Trong CSS Flexbox, thuộc tính nào canh phần tử con theo trục chính?",
    options: [
      "A. align-items",
      "B. align-content",
      "C. justify-content",
      "D. place-items",
    ],
    correctIndex: 2,
  },
  {
    question:
      "Python: Kết quả của `len({1, 1, 2, 3})` là bao nhiêu?",
    options: ["A. 2", "B. 3", "C. 4", "D. 1"],
    correctIndex: 1,
  },
];

let currentQuestion = 0;
let score = 0;

const questionNumberEl = document.getElementById("question-number");
const scoreEl = document.getElementById("score");
const questionTextEl = document.getElementById("question-text");
const optionsEl = document.getElementById("options");
const feedbackEl = document.getElementById("feedback");
const nextButtonEl = document.getElementById("next-button");

function renderQuestion() {
  const item = quizData[currentQuestion];

  questionNumberEl.textContent = `Câu ${currentQuestion + 1}/${quizData.length}`;
  scoreEl.textContent = `Điểm: ${score}`;
  questionTextEl.textContent = item.question;
  optionsEl.innerHTML = "";
  feedbackEl.textContent = "";
  feedbackEl.className = "feedback";
  nextButtonEl.disabled = true;

  item.options.forEach((optionText, index) => {
    const button = document.createElement("button");
    button.className = "option-btn";
    button.type = "button";
    button.textContent = optionText;
    button.addEventListener("click", () => handleAnswer(index));
    optionsEl.appendChild(button);
  });
}

function handleAnswer(selectedIndex) {
  const item = quizData[currentQuestion];
  const optionButtons = Array.from(optionsEl.querySelectorAll(".option-btn"));

  optionButtons.forEach((btn) => {
    btn.disabled = true;
  });

  const isCorrect = selectedIndex === item.correctIndex;

  if (isCorrect) {
    score += 1;
    optionButtons[selectedIndex].classList.add("correct");
    feedbackEl.textContent = "✅ Chính xác!";
    feedbackEl.classList.add("correct");
  } else {
    optionButtons[selectedIndex].classList.add("wrong");
    optionButtons[item.correctIndex].classList.add("correct");

    const rightAnswerText = item.options[item.correctIndex];
    feedbackEl.textContent = `❌ Sai rồi. Đáp án đúng là: ${rightAnswerText}`;
    feedbackEl.classList.add("wrong");
  }

  scoreEl.textContent = `Điểm: ${score}`;
  nextButtonEl.disabled = false;
}

function handleNextQuestion() {
  if (currentQuestion < quizData.length - 1) {
    currentQuestion += 1;
    renderQuestion();
    return;
  }

  questionNumberEl.textContent = "Hoàn thành";
  questionTextEl.textContent = `Bạn đạt ${score}/${quizData.length} điểm.`;
  optionsEl.innerHTML = "";
  feedbackEl.textContent = "Nhấn nút bên dưới để chơi lại.";
  feedbackEl.className = "feedback";
  nextButtonEl.textContent = "Chơi lại";
}

nextButtonEl.addEventListener("click", () => {
  if (currentQuestion === quizData.length - 1 && nextButtonEl.textContent === "Chơi lại") {
    currentQuestion = 0;
    score = 0;
    nextButtonEl.textContent = "Câu tiếp theo";
    renderQuestion();
    return;
  }

  handleNextQuestion();
});

renderQuestion();
