set(SQUEEZE_HEADERS squeeze.h
                    sqzclass.h
                    sqzclosure.h
                    sqzdef.h
                    sqzimpl.h
                    sqzobject.h
                    sqzscript.h
                    sqzscript.h
                    sqzstackop.h
                    sqztable.h
                    sqztableimpl.h
                    sqzutil.h
                    sqzvm.h)

add_custom_target(squeeze SOURCES ${SQUEEZE_HEADERS})

install(FILES ${SQUEEZE_HEADERS} DESTINATION include)