/*

Trabalho 01 da Disciplina de Análise e Projeto de Software

Autor: Miguel Miron Silva

*/


//Gerados pelo Gemini
#include <iostream>
#include <vector>
#include <string>
#include <memory>
//Adicionados pelo Kimi
#include <stack>
#include <chrono>
#include <stdexcept>

using namespace std;

// --- SINGLETON (Timer) ---
//Modernizado pelo Kimi
class ExamTimer {
private:
    chrono::steady_clock::time_point start; // chrono ainda precisa do prefixo ou de outro using
    chrono::seconds duration;
    ExamTimer() : start(chrono::steady_clock::now()), duration(3600) {}
public:
    static ExamTimer& getInstance() {
        static ExamTimer instance;
        return instance;
    }
    int getRemainingSeconds() {
        auto elapsed = chrono::steady_clock::now() - start;
        auto remaining = duration -
               chrono::duration_cast<chrono::seconds>(elapsed);
        return static_cast<int>(remaining.count()) > 0 ?
               static_cast<int>(remaining.count()) : 0;
    }
};

// --- STRATEGY (Correção) ---
class GradingStrategy {
public:
    virtual bool grade(string answer, string key) = 0;
    virtual ~GradingStrategy() = default;
};

class ExactMatchStrategy : public GradingStrategy {
public:
    bool grade(string answer, string key) override {
        return answer == key;
    }
};


// --- COMPOSITE (Estrutural) ---
//Modernizado pelo Kimi - Folha, Composto e Iterator adicionados
class ExamComponent {
public:
    virtual void display(int depth = 0) const = 0;
    virtual void add(shared_ptr<ExamComponent> c) {
        throw runtime_error("Incapaz de adicionar folha!"); 
    }
    virtual ~ExamComponent() = default;
};


// --- PRODUTOS ---
// Folha
class Question : public ExamComponent {
    string text;
    string key;
    shared_ptr<GradingStrategy> grader;
public:
    Question(const string& t,
             const string& k,
             shared_ptr<GradingStrategy> g)
        : text(t), key(k), grader(move(g)) {} // move ao invés de std::move

    void display(int depth = 0) const override {
        string indent(depth * 2, ' ');
        cout << indent << "Questao: " << text << "\n";
    }

    bool checkAnswer(const string& ans) const {
        return grader->grade(ans, key);
    }
};

// Composto
class ExamSection : public ExamComponent {
    vector<shared_ptr<ExamComponent>> children;
    string title;
public:
    explicit ExamSection(const string& t) : title(t) {}

    void add(shared_ptr<ExamComponent> c) override {
        children.push_back(move(c));
    }

    void display(int depth = 0) const override {
        string indent(depth * 2, ' ');
        cout << indent << "--- SECAO: " << title << " ---\n";
        for (const auto& c : children) c->display(depth + 1);
    }

    // Iterator real (oculta estrutura interna)
    class Iterator {
        using Iter = vector<shared_ptr<ExamComponent>>::iterator;
        Iter curr, end;
    public:
        Iterator(vector<shared_ptr<ExamComponent>>& vec)
            : curr(vec.begin()), end(vec.end()) {}
        bool hasNext() { return curr != end; }
        shared_ptr<ExamComponent> next() {
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
    static shared_ptr<Question> createMultipleChoice(string text, string key) {
        return make_shared<Question>(text + " (A/B/C/D)", key, make_shared<ExactMatchStrategy>());
    }
};

// --- MEMENTO (Estado) ---
//Modernizado pelo Kimi - adicionado respostas
class ExamMemento {
    int currentIndex;
    vector<string> answers;
    chrono::steady_clock::time_point ts;
public:
    ExamMemento(int idx,
                vector<string> ans)
        : currentIndex(idx),
          answers(move(ans)),
          ts(chrono::steady_clock::now()) {}

    int getIndex() const { return currentIndex; }
    const vector<string>& getAnswers() const {
        return answers;
    }
};


// ---SESSION ---
//Modernizado pelo Kimi - adicionado respostas
class ExamSession {
    int currentIndex = 0;
    vector<string> answers;
public:
    void answerQuestion(const string& ans) {
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
    shared_ptr<ExamSection> root;
    ExamComponent* currentScope; 
public:
    ExamBuilder(const string& title) {
        root = make_shared<ExamSection>(title);
        currentScope = root.get();
    }
    ExamBuilder& addSection(const string& name) {
        auto sec = make_shared<ExamSection>(name);
        currentScope->add(sec);
        return *this;
    }
    ExamBuilder& addQuestion(const string& text,
                             const string& key) {
        currentScope->add(QuestionFactory::createMultipleChoice(text, key));
        return *this;
    }
    shared_ptr<ExamSection> build() { return root; }
};


// --- CLIENTE ---
//Modernizado pelo Kimi
int main() {
   //Singleton
    auto& timer = ExamTimer::getInstance();
    cout << "Tempo restante: " << timer.getRemainingSeconds() << "s\n";

    //Builder + Composite
    ExamBuilder builder("Prova Final de C++");
    auto prova = builder
        .addSection("Logica")
        .addQuestion("Quanto e 2+2?", "4")
        .addQuestion("Quanto e 3*3?", "9")
        .addSection("Orientacao a Objetos")
        .addQuestion("O que e polimorfismo?", "Muitas formas")
        .build();

    //Display
    prova->display();

    //terator
    cout << "\nPercorrendo com Iterator:\n";
    auto it = dynamic_cast<ExamSection*>(prova.get())
                  ->createIterator();
    while (it.hasNext()) {
        auto comp = it.next();
        comp->display();
    }

    //Session + Memento
    ExamSession session;
    session.answerQuestion("4");
    session.answerQuestion("10"); // errado
    auto checkpoint = session.save();

    cout << "\n[Sistema caiu... Restaurando...]\n";
    session.restore(checkpoint);
    cout << "Restaurado para indice: " << session.getCurrentIndex() << "\n";
    
    return 0;
}
