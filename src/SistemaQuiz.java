import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

/*
Trabalho 01 da Disciplina de Análise e Projeto de Software

Autor: Miguel Miron Silva

Versão Java
*/

public class SistemaQuiz {

    // --- STRATEGY ---
    interface GradingStrategy {
        boolean grade(String answer, String key);
    }

    static class ExactMatchStrategy implements GradingStrategy {
        @Override
        public boolean grade(String answer, String key) {
            return answer.equals(key);
        }
    }

    // --- COMPOSITE ---
    static abstract class ExamComponent {
        public abstract void display(int depth);

        public void add(ExamComponent c) {
            throw new UnsupportedOperationException("Incapaz de adicionar em uma folha!");
        }
    }

    // --- PRODUTOS ---
    // Folha
    static class Question extends ExamComponent {
        private String text;
        private String key;
        private GradingStrategy grader;

        public Question(String text, String key, GradingStrategy grader) {
            this.text = text;
            this.key = key;
            this.grader = grader;
        }

        @Override
        public void display(int depth) {
            String indent = "  ".repeat(depth);
            System.out.println(indent + "Questao: " + text);
        }

        public boolean checkAnswer(String ans) {
            return grader.grade(ans, key);
        }
    }

    // Composto
    static class ExamSection extends ExamComponent {
        private List<ExamComponent> children = new ArrayList<>();
        private String title;

        public ExamSection(String title) {
            this.title = title;
        }

        @Override
        public void add(ExamComponent c) {
            children.add(c);
        }

        @Override
        public void display(int depth) {
            String indent = "  ".repeat(depth);
            System.out.println(indent + "--- SECAO: " + title + " ---");
            for (ExamComponent c : children) {
                c.display(depth + 1);
            }
        }

        // Iterator
        public ComponentIterator createIterator() {
            return new ComponentIterator(children);
        }

        // Inner class para o Iterator
        static class ComponentIterator {
            private Iterator<ExamComponent> iterator;

            public ComponentIterator(List<ExamComponent> list) {
                this.iterator = list.iterator();
            }

            public boolean hasNext() {
                return iterator.hasNext();
            }

            public ExamComponent next() {
                return iterator.next();
            }
        }
    }

    // --- FACTORY ---
    static class QuestionFactory {
        public static Question createMultipleChoice(String text, String key) {
            // Em Java, passamos a instância da estratégia
            return new Question(text + " (A/B/C/D)", key, new ExactMatchStrategy());
        }
    }

    // --- MEMENTO ---
    static class ExamMemento {
        private final int currentIndex;
        private final List<String> answers;

        public ExamMemento(int idx, List<String> ans) {
            this.currentIndex = idx;
            this.answers = new ArrayList<>(ans);
        }

        public int getIndex() {
            return currentIndex;
        }

        public List<String> getAnswers() {
            return answers;
        }
    }

    // --- SESSION  ---
    static class ExamSession {
        private int currentIndex = 0;
        private List<String> answers = new ArrayList<>();

        public void answerQuestion(String ans) {
            if (currentIndex >= answers.size()) {
                answers.add(ans);
            } else {
                answers.set(currentIndex, ans);
            }
            currentIndex++;
        }
        
        // Pega respostas
        public String getAnswer(int index) {
            if (index >= 0 && index < answers.size()) {
                return answers.get(index);
            }
            return "(Sem resposta)";
        }

        public ExamMemento save() {
            return new ExamMemento(currentIndex, answers);
        }

        public void restore(ExamMemento m) {
            this.currentIndex = m.getIndex();
            this.answers = new ArrayList<>(m.getAnswers());
        }

        public int getCurrentIndex() {
            return currentIndex;
        }
    }

    // --- BUILDER ---
    static class ExamBuilder {
        private ExamSection root;
        private ExamComponent currentScope;

        public ExamBuilder(String title) {
            root = new ExamSection(title);
            currentScope = root;
        }

        public ExamBuilder addSection(String name) {
            ExamSection sec = new ExamSection(name);
            currentScope.add(sec);
            return this;
        }

        public ExamBuilder addQuestion(String text, String key) {
            currentScope.add(QuestionFactory.createMultipleChoice(text, key));
            return this;
        }

        public ExamSection build() {
            return root;
        }
    }

    // --- CLIENTE  ---
    public static void main(String[] args) {
        String resposta;

        // Builder + Composite
        ExamBuilder builder = new ExamBuilder("Prova Final de Java");
        ExamSection prova = builder
                .addSection("Logica")
                .addQuestion("Quanto e 2+2?", "4")
                .addQuestion("Quanto e 3*3?", "9")
                .addSection("Orientacao a Objetos")
                .addQuestion("O que e polimorfismo?", "Muitas formas")
                .build();

        // Display (Composite)
        prova.display(0);


        // Iterator
        System.out.println("\nPercorrendo com Iterator:");
        ExamSection.ComponentIterator it = prova.createIterator();
        while (it.hasNext()) {
            ExamComponent comp = it.next();
            comp.display(0);
        }

        // Session + Memento
        ExamSession session = new ExamSession();
        session.answerQuestion("4");
        session.answerQuestion("10"); // resposta errada proposital
        
        System.out.println("\n[Salvando estado...]");
        ExamMemento checkpoint = session.save();

        System.out.println("[Sistema caiu... Restaurando...]");
        session.restore(checkpoint);
        System.out.println("Restaurado para indice: " + session.getCurrentIndex());

        System.out.println("Retornando respostas: ");
        for(int i = 0; i < session.getCurrentIndex(); i++){
            resposta = session.getAnswer(i);
            System.out.println(resposta);
        }
        
    }
}
