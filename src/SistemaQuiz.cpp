//Gerados pelo Gemini
#include <iostream>
#include <vector>
#include <string>
#include <memory>
//Adicionados pelo Kimi
#include <stack>
#include <chrono>
#include <stdexcept>

// --- SINGLETON (Timer) ---
//Modernizado pelo Kimi
class ExamTimer {
private:
    std::chrono::steady_clock::time_point start;
    std::chrono::seconds duration;
    ExamTimer() : start(std::chrono::steady_clock::now()), duration(3600) {}
public:
    static ExamTimer& getInstance() {
        static ExamTimer instance;
        return instance;
    }
    int getRemainingSeconds() {
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto remaining = duration -
               std::chrono::duration_cast<std::chrono::seconds>(elapsed);
        return static_cast<int>(remaining.count()) > 0 ?
               static_cast<int>(remaining.count()) : 0;
    }
};

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

// --- COMPOSITE (Estrutural) ---
//Modernizado pelo Kimi - Folha, Composto e Iterator adicionados
class ExamComponent {
public:
    virtual void display(int depth = 0) const = 0;
    virtual void add(std::shared_ptr<ExamComponent> c) {
        throw std::runtime_error("Incapaz de adicionar folha!"); //Adicão do Kimi
    }
    virtual ~ExamComponent() = default;
};


// --- PRODUTOS ---
// Folha
class Question : public ExamComponent {
    std::string text;
    std::string key;
    std::shared_ptr<GradingStrategy> grader;
public:
    Question(const std::string& t,
             const std::string& k,
             std::shared_ptr<GradingStrategy> g)
        : text(t), key(k), grader(std::move(g)) {}

    void display(int depth = 0) const override {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "Questao: " << text << "\n";
    }

    bool checkAnswer(const std::string& ans) const {
        return grader->grade(ans, key);
    }
};

// Composto
class ExamSection : public ExamComponent {
    std::vector<std::shared_ptr<ExamComponent>> children;
    std::string title;
public:
    explicit ExamSection(const std::string& t) : title(t) {}

    void add(std::shared_ptr<ExamComponent> c) override {
        children.push_back(std::move(c));
    }

    void display(int depth = 0) const override {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "--- SECAO: " << title << " ---\n";
        for (const auto& c : children) c->display(depth + 1);
    }

    // Iterator real (oculta estrutura interna)
    class Iterator {
        using Iter = std::vector<std::shared_ptr<ExamComponent>>::iterator;
        Iter curr, end;
    public:
        Iterator(std::vector<std::shared_ptr<ExamComponent>>& vec)
            : curr(vec.begin()), end(vec.end()) {}
        bool hasNext() { return curr != end; }
        std::shared_ptr<ExamComponent> next() {
            if (!hasNext()) return nullptr;
            return *curr++;
        }
    };

    Iterator createIterator() {
        return Iterator(children);
    }
};


// --- FACTORY METHOD ---
class QuestionFactory {
public:
    static std::shared_ptr<Question> createMultipleChoice(std::string text, std::string key) {
        return std::make_shared<Question>(text + " (A/B/C/D)", key, std::make_shared<ExactMatchStrategy>());
    }
    // Outros métodos de fábrica (TrueFalse, etc)...
};

// --- MEMENTO (Estado) ---
//Modernizado pelo Kimi - adicionado respostas
class ExamMemento {
    int currentIndex;
    std::vector<std::string> answers;
    std::chrono::steady_clock::time_point ts;
public:
    ExamMemento(int idx,
                std::vector<std::string> ans)
        : currentIndex(idx),
          answers(std::move(ans)),
          ts(std::chrono::steady_clock::now()) {}

    int getIndex() const { return currentIndex; }
    const std::vector<std::string>& getAnswers() const {
        return answers;
    }
};


// ---SESSION ---
//Modernizado pelo Kimi - adicionado respostas
class ExamSession {
    int currentIndex = 0;
    std::vector<std::string> answers;
public:
    void answerQuestion(const std::string& ans) {
        if (currentIndex >= answers.size())
            answers.push_back(ans);
        else
            answers[currentIndex] = ans;
        ++currentIndex;
    }

    ExamMemento save() {
        return ExamMemento(currentIndex, answers);
    }

    void restore(const ExamMemento& m) {
        currentIndex = m.getIndex();
        answers = m.getAnswers();
    }

    int getCurrentIndex() const { return currentIndex; }
};


// --- BUILDER  ---
//Modificado pelo Gemini para gerar árvore
class ExamBuilder {
    std::shared_ptr<ExamSection> root;
    ExamComponent* currentScope; // Ponteiro bruto não-proprietário para navegação
public:
    ExamBuilder(const std::string& title) {
        root = std::make_shared<ExamSection>(title);
        currentScope = root.get();
    }
    ExamBuilder& addSection(const std::string& name) {
        auto sec = std::make_shared<ExamSection>(name);
        currentScope->add(sec);
        return *this;
    }
    ExamBuilder& addQuestion(const std::string& text,
                             const std::string& key) {
        currentScope->add(QuestionFactory::createMultipleChoice(text, key));
        return *this;
    }
    std::shared_ptr<ExamSection> build() { return root; }
};


// --- CLIENTE ---
//Modernizado pelo Kimi
int main() {
    // 1. Singleton
    auto& timer = ExamTimer::getInstance();
    std::cout << "Tempo restante: " << timer.getRemainingSeconds() << "s\n";

    // 2. Builder + Composite
    ExamBuilder builder("Prova Final de C++");
    auto prova = builder
        .addSection("Logica")
        .addQuestion("Quanto e 2+2?", "4")
        .addQuestion("Quanto e 3*3?", "9")
        .addSection("Orientacao a Objetos")
        .addQuestion("O que e polimorfismo?", "Muitas formas")
        .build();

    // 3. Display
    prova->display();

    // 4. Iterator
    std::cout << "\nPercorrendo com Iterator:\n";
    auto it = dynamic_cast<ExamSection*>(prova.get())
                  ->createIterator();
    while (it.hasNext()) {
        auto comp = it.next();
        comp->display();
    }

    // 5. Session + Memento
    ExamSession session;
    session.answerQuestion("4");
    session.answerQuestion("10"); // errado
    auto checkpoint = session.save();

    std::cout << "\n[Sistema caiu... Restaurando...]\n";
    session.restore(checkpoint);
    std::cout << "Restaurado para indice: " << session.getCurrentIndex() << "\n";
    
    return 0;
}
