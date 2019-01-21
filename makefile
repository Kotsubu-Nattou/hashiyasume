#################  Makefile�̎g����  ####################
# ���ۂɋL�q����̂� ### �L������ ### �̍��ځB
# CFLAFS�ŃR���p�C���I�v�V�������w��B
# .o�t�@�C������������.h�t�@�C���̈ˑ��֌W���ύX�ɂȂ����ꍇ�A
# "�w�b�_�t�@�C���̈ˑ��֌W���`"�̍��ڂ�ҏW����B
# ����̃w�b�_���g�p���Ȃ��ꍇ�A���̍��ڂ͕K�v�Ȃ��B
########################################################


# �}�N�����`  ### �L������ ###
PROG   = a.exe
OBJS   = a.o resource.o
LIBS   = -lopengl32 -lglu32 -lglew32 -lglfw3dll
COMP   = g++
CFLAGS = -g -Wall


# �f�t�H���g�^�[�Q�b�g�i�擪�ɂ���̂Ńf�t�H���g�Ŏ��s�j
#.PHONY: all
#all: clean $(PROG)


# �����J�̃^�[�Q�b�g
$(PROG): $(OBJS)
	$(COMP) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROG)


# �T�t�B�b�N�X���[���i.o���R���p�C������Ƃ��̓R�`���j
.c.o:
	$(COMP) $(CFLAGS) -c $<

.cpp.o:
	$(COMP) $(CFLAGS) -c $<


# ���ԃt�@�C���폜�p�^�[�Q�b�g
.PHONY: clean
clean:
	-rm *.o


# �w�b�_�t�@�C���̈ˑ��֌W���A�ʂɒ�`  ### �L������ ###
#main.o: sub.h
