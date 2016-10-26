#
# Copyright (C) 2016 Freescale Semiconductor, Inc.
# All rights reserved.
#
# This software may be distributed under the terms of the
# GNU General Public License ("GPL") as published by the Free Software
# Foundation, either version 2 of that License or (at your option) any
# later version.
#
# THIS SOFTWARE IS PROVIDED BY Freescale Semiconductor ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Freescale Semiconductor BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# CONFIG can be one of:
# RTOS (for RTOS/Linux on the same board)
# EP or RC (for Linux over PCIE)
CONFIG ?= RTOS

ifeq ($(CONFIG),RTOS)
SRC := $(shell pwd)/release/iccs32v234/sample/linux_app
MODULE_SRC := $(SRC)/ICC_module
SAMPLE_MODULE_SRC := $(SRC)/ICC_Sample_module
else
SRC := $(shell pwd)/release/iccs32v234/sample/linux2linux_app
MODULE_SRC := $(SRC)/ICC_module_$(CONFIG)
SAMPLE_MODULE_SRC := $(SRC)/ICC_Sample_module_$(CONFIG)
endif


.PHONY: build modules_install clean

all: build

build:
	$(MAKE) -C $(MODULE_SRC)
	$(MAKE) -C $(SAMPLE_MODULE_SRC)

modules_install:
	$(MAKE) -C $(MODULE_SRC) modules_install
	$(MAKE) -C $(SAMPLE_MODULE_SRC) modules_install

clean:
	$(MAKE) -C $(MODULE_SRC) clean
	$(MAKE) -C $(SAMPLE_MODULE_SRC) clean
