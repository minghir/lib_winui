# 1. Configurări de bază
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -fPIC -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00

# Căile de include (adaugă -I pentru folderul tău de include dacă ai unul separat)
CXXFLAGS += -I./include -I../../thirdparty/pugixml-1.15/src -I../../thirdparty/mupdf/include -I../../thirdparty/PDF-Writer/PDFWriter -I../../thirdparty/PDF-Writer/FreeType/include

# Numele librăriei finale (pe Windows/MinGW este bine să aibă extensia .a pentru GCC)
TARGET = lib_winui.a

# 2. DETECTAREA AUTOMATĂ ȘI RECURSIVĂ A FIȘIERELOR
# Comanda 'find' va căuta în folderul 'src' și în TOATE subfolderele lui după fișiere .cpp
SRCS = $(shell find src -path "src/backup" -prune -o -name "*.cpp" -type f -print)

# Mapăm automat fiecare fișier .cpp la un fișier obiect .o
# Toate fișierele .o vor fi create exact în subfolderul unde se află codul sursă
OBJS = $(SRCS:.cpp=.o)

# 3. REGULILE MAIN
all: $(TARGET)

# Regula pentru crearea librăriei statice din fișierele obiect (.o)
# Utilitarul 'ar' (archiver) le împachetează împreună. 'rcs' creează și indexează librăria.
$(TARGET): $(OBJS)
	@echo "[LINKER] Se creeaza libraria statica: $(TARGET)"
	ar rcs $@ $(OBJS)
	@echo "[SUCCESS] Libraria a fost generata cu succes!"

# Regula generică pentru a compila fiecare fișier .cpp în fișierul său .o corespunzător
%.o: %.cpp
	@echo "[COMPILER] Se compileaza: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regula de curățenie: șterge toate fișierele .o și librăria finală
clean:
	@echo "[CLEAN] Se curata fisierele obiect si libraria..."
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean