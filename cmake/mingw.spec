*libgcc:
%{mthreads:-lmingwthrd} -lmingw32    %{shared-libgcc:-lgcc_s} %{!shared-libgcc:-lgcc_eh}    -lgcc    -lmoldname90 -lmingwex -lmsvcr90

