#################  Makefileの使い方  ####################
# 実際に記述するのは ### 記入項目 ### の項目。
# PROG   --- Windows環境下では、必ず「.exe」を付けること
# RESRC  --- リソースファイルを使用するなら「ファイル名.o」
# CFLAFS --- コンパイルオプションを指定
# .oファイルが増えたり.hファイルの依存関係が変更になった場合、
# 「ヘッダファイルの依存関係」の項目で、個別に指定する。
# なお、自作のヘッダを使用しない場合、この項目は必要なし。
########################################################


# マクロ   ### 記入項目 ###
PROG   = a.exe
RESRC  = resource.o
OBJS   = a.o $(RESRC)
LIBS   = -lopengl32 -lglu32 -lglew32 -lglfw3dll
COMP   = g++
CFLAGS = -g -Wall


# デフォルトターゲット（先頭にあるのでデフォルトで実行）
#.PHONY: all
#all: clean $(PROG)


# ターゲット：リンカ
$(PROG): $(OBJS)
	$(COMP) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROG)


# ターゲット：リソースファイルのコンパイル
$(RESRC):
	windres $*.rc -o $@


# ターゲット：中間ファイルの削除
.PHONY: clean
clean:
	-rm *.o


# サフィックスルール（.oをコンパイルするときはコチラ）
.c.o:
	$(COMP) $(CFLAGS) -c $<

.cpp.o:
	$(COMP) $(CFLAGS) -c $<


# ヘッダファイルの依存関係   ### 記入項目 ###
#main.o: sub.h
