################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
actions_k_nn.obj: ../actions_k_nn.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=large --use_hw_mpy=F5 --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccsv7/ccs_base/msp430/include" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR5994__ --define=_MPU_ENABLE -g --printf_support=full --diag_suppress=10420 --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="actions_k_nn.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

intermittent_learning.obj: ../intermittent_learning.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=large --use_hw_mpy=F5 --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccsv7/ccs_base/msp430/include" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR5994__ --define=_MPU_ENABLE -g --printf_support=full --diag_suppress=10420 --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="intermittent_learning.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

memory_util.obj: ../memory_util.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=large --use_hw_mpy=F5 --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccsv7/ccs_base/msp430/include" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/Seulki/workspace_v7/intermittent_k_nn_acc" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR5994__ --define=_MPU_ENABLE -g --printf_support=full --diag_suppress=10420 --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="memory_util.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


