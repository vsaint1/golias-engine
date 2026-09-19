class PythonBehavior:

    def start(self):              pass
    def update(self, delta_time): pass
    def on_enable(self):          pass
    def on_disable(self):         pass
    def on_destroy(self):         pass

    @property
    def transform(self):
        return self.get_transform()

    # `self` proxies the owning GameObject: self.get_name(), self.set_active(...), ...
    def __getattr__(self, name):
        owner = self.__dict__.get("_owner", None)
        if owner is None:
            raise AttributeError(name)
        return getattr(owner, name)

golias.engine.PythonBehavior = PythonBehavior
