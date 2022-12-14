#
# (c) Copyright 2016 Freescale Semiconductor, Inc.
# (c) Copyright 2016 NXP
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
# EP for EndPoint (s32v234 pcie)
# RC for RootComplex (s32v234 evb or ls208xx rdb)
CONFIG ?= RTOS

SRC := $(shell pwd)/release/iccs32v234/sample/linux_app

MODULE_SRC := $(SRC)/ICC_module
SAMPLE_MODULE_SRC := $(SRC)/ICC_Sample_module

.PHONY: build modules_install clean

all: build

build:
	$(MAKE) -C $(MODULE_SRC)
	$(MAKE) -C $(SAMPLE_MODULE_SRC)

modules_install:
	$(MAKE) -C $(MODULE_SRC) modules_install
	$(MAKE) -C $(SAMPLE_MODULE_SRC) modules_install
ifeq ($(CONFIG),RTOS)
	$(MAKE) -C $(MODULE_SRC) m4_install
endif

headers_install:
	$(MAKE) -C $(MODULE_SRC) headers_install

clean:
	$(MAKE) -C $(MODULE_SRC) clean
	$(MAKE) -C $(SAMPLE_MODULE_SRC) clean
