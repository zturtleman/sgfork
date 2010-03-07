@cd baseq3 || echo Error no baseq3 && goto quit
@move * ../smokinguns || echo Error!!! can not move to smokinguns dir && goto quit
@cd ../
@move smokinguns base || echo Error!!! can not move smokinguns to base dir && goto quit
@rd baseq3 || echo Error!!! can not delete baseq3 dir && goto quit
@cd base
@move sg_pak0.pk3 pak1.pk3 || echo Error!!! sg_pak0.pk3 to pak1.pk3 && goto quit

@echo You configs will be stored there:
@echo %appdata%/SmokinGuns/base
@echo move there your current configs, demos, screenshots if any

@echo Successful!
:quit
@pause