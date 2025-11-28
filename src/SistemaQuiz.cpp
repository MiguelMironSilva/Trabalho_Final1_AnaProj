#include <iostream>
#include <vector>
#include <string>
#include <memory>

// --- SINGLETON (Timer) ---
class ExamTimer {
private:
    static ExamTimer* instance;
    int secondsRemaining;
    ExamTimer() : secondsRemaining(3600) {} // 1 hora
public:
    static ExamTimer* getInstance() {
        if (instance == nullptr) instance = new ExamTimer();
        return instance;
    }
    int getTime() { return secondsRemaining--; }
};
ExamTimer* ExamTimer::instance = nullptr;

// --- STRATEGY (Correção) ---
class GradingStrategy {
public:
    virtual bool grade(std::string answer, std::string key) = 0;
    virtual ~GradingStrategy() = default;
};

class ExactMatchStrategy : public GradingStrategy {
public:
    bool grade(std::string answer, std::string key) override {
        return answer == key;
    }
};

// --- COMPOSITE (Estrutural - Adição sugerida) ---
class ExamComponent {
public:
    virtual void display() = 0;
    virtual void add(std::shared_ptr<ExamComponent> c) {}
    virtual ~ExamComponent() = default;
};

// --- FACTORY METHOD & PRODUTOS ---
class Question : public ExamComponent {
protected:
    std::string text;
    std::string answerKey;
    std::shared_ptr<GradingStrategy> grader;
public:
    Question(std::string t, std::string k, std::shared_ptr<GradingStrategy> g) 
        : text(t), answerKey(k), grader(g) {}
    
    void display() override {
        std::cout << "Questao: " << text << std::endl;
    }
    
    bool checkAnswer(std::string studentAnswer) {
        return grader->grade(studentAnswer, answerKey);
    }
};

class QuestionFactory {
public:
    static std::shared_ptr<Question> createMultipleChoice(std::string text, std::string key) {
        return std::make_shared<Question>(text + " (A/B/C/D)", key, std::make_shared<ExactMatchStrategy>());
    }
    // Outros métodos de fábrica (TrueFalse, etc)...
};

class ExamSection : public ExamComponent { // O Composite
    std::vector<std::shared_ptr<ExamComponent>> children;
    std::string title;
public:
    ExamSection(std::string t) : title(t) {}
    void add(std::shared_ptr<ExamComponent> c) override {
        children.push_back(c);
    }
    void display() override {
        std::cout << "\n--- SECAO: " << title << " ---" << std::endl;
        for (auto& c : children) c->display();
    }
    // Método para o Iterator acessar os filhos seria implementado aqui
    auto getChildren() { return children; } 
};

// --- MEMENTO (Estado) ---
class ExamMemento {
    int currentQuestionIndex;
    // Poderia salvar respostas também
public:
    ExamMemento(int index) : currentQuestionIndex(index) {}
    int getState() { return currentQuestionIndex; }
};

class ExamSession {
    int currentIndex = 0;
public:
    void answerQuestion() {
        currentIndex++;
        // Lógica de responder...
    }
    ExamMemento save() {
        return ExamMemento(currentIndex);
    }
    void restore(ExamMemento m) {
        currentIndex = m.getState();
    }
};

// --- CLIENTE ---
int main() {
    // 1. Singleton
    ExamTimer* timer = ExamTimer::getInstance();
    std::cout << "Tempo restante: " << timer->getTime() << "s\n";

    // 2. Factory & Composite (Estrutura)
    auto prova = std::make_shared<ExamSection>("Prova Final de C++");
    
    auto secaoLogica = std::make_shared<ExamSection>("Logica");
    secaoLogica->add(QuestionFactory::createMultipleChoice("Quanto e 2+2?", "4"));
    
    auto secaoOO = std::make_shared<ExamSection>("Orientacao a Objetos");
    secaoOO->add(QuestionFactory::createMultipleChoice("O que e Polimorfismo?", "Muitas formas"));

    prova->add(secaoLogica);
    prova->add(secaoOO);

    // 3. Exibição (Composite em ação)
    prova->display();

    // 4. Memento (Simulação)
    ExamSession session;
    session.answerQuestion(); // Respondeu a primeira
    ExamMemento checkpoint = session.save(); // Checkpoint salvo (índice 1)
    
    std::cout << "\n[Sistema caiu... Restaurando...]\n";
    session.restore(checkpoint);
    
    return 0;
}
