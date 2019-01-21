#################  Makefileの使い方  ####################
# 実際に記述するのは ### 記入項目 ### の項目。
# CFLAFSでコンパイルオプションを指定。
# .oファイルが増えたり.hファイルの依存関係が変更になった場合、
# "ヘッダファイルの依存関係を定義"の項目を編集する。
# 自作のヘッダを使用しない場合、この項目は必要なし。
########################################################


# マクロを定義  ### 記入項目 ###
PROG   = a.exe
OBJS   = a.o resource.o
LIBS   = -lopengl32 -lglu32 -lglew32 -lglfw3dll
COMP   = g++
CFLAGS = -g -Wall


# デフォルトターゲット（先頭にあるのでデフォルトで実行）
#.PHONY: all
#all: clean $(PROG)


# リンカのターゲット
$(PROG): $(OBJS)
	$(COMP) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROG)


# サフィックスルール（.oをコンパイルするときはコチラ）
.c.o:
	$(COMP) $(CFLAGS) -c $<

.cpp.o:
	$(COMP) $(CFLAGS) -c $<


# 中間ファイル削除用ターゲット
.PHONY: clean
clean:
	-rm *.o


# ヘッダファイルの依存関係を、個別に定義  ### 記入項目 ###
#main.o: sub.h
