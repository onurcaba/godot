def can_build(env, platform):
    return env.editor_build


def configure(env):
    env.Append(CPPDEFINES=["CBT_STUDIO"])


def get_doc_classes():
    return []


def get_doc_path():
    return "doc_classes"
