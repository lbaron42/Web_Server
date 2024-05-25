from locust import HttpUser, between, task


class WebsiteUser(HttpUser):
    wait_time = between(5, 15)
    
    def on_start(self):
        pass
    
    @task
    def index(self):
        self.client.get("/")
    # @task
    # def index(self):
    #     self.client.head("/big_file.test")

