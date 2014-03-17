#
#

# Abre o menu com as opções
all:
	scripts/main.sh "$(CURDIR)" | tee build.log


# Remove os arquivos temporários
clean:
	sudo rm -rf tmp
	sudo rm -rf logs
	sudo rm -rf bootstrap/base
	sudo rm -rf build.log


# Remove os arquivos temporários e as ISOs geradas
cleanup: clean
	rm -rf iso

