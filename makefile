#################  Makefile�̎g����  ####################
# ���ۂɋL�q����̂� ### �L������ ### �̍��ځB
# PROG   --- Windows�����ł́A�K���u.exe�v��t���邱��
# RESRC  --- ���\�[�X�t�@�C�����g�p����Ȃ�u�t�@�C����.o�v
# CFLAFS --- �R���p�C���I�v�V�������w��
# .o�t�@�C������������.h�t�@�C���̈ˑ��֌W���ύX�ɂȂ����ꍇ�A
# �u�w�b�_�t�@�C���̈ˑ��֌W�v�̍��ڂŁA�ʂɎw�肷��B
# �Ȃ��A����̃w�b�_���g�p���Ȃ��ꍇ�A���̍��ڂ͕K�v�Ȃ��B
########################################################


# �}�N��   ### �L������ ###
PROG   = a.exe
RESRC  = resource.o
OBJS   = a.o $(RESRC)
LIBS   = -lopengl32 -lglu32 -lglew32 -lglfw3dll
COMP   = g++
CFLAGS = -g -Wall


# �f�t�H���g�^�[�Q�b�g�i�擪�ɂ���̂Ńf�t�H���g�Ŏ��s�j
#.PHONY: all
#all: clean $(PROG)


# �^�[�Q�b�g�F�����J
$(PROG): $(OBJS)
	$(COMP) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROG)


# �^�[�Q�b�g�F���\�[�X�t�@�C���̃R���p�C��
$(RESRC):
	windres $*.rc -o $@


# �^�[�Q�b�g�F���ԃt�@�C���̍폜
.PHONY: clean
clean:
	-rm *.o


# �T�t�B�b�N�X���[���i.o���R���p�C������Ƃ��̓R�`���j
.c.o:
	$(COMP) $(CFLAGS) -c $<

.cpp.o:
	$(COMP) $(CFLAGS) -c $<


# �w�b�_�t�@�C���̈ˑ��֌W   ### �L������ ###
#main.o: sub.h
