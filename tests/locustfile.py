from locust import HttpUser, between, task


class WebsiteUser(HttpUser):
    wait_time = between(2, 6)
    
    def on_start(self):
        pass

    @task(10)
    def index(self):
        self.client.get("/")

    @task(10)
    def index(self):
        self.client.get("/action.html")

    # @task(5)
    # def big_file(self):
    #     self.client.head("/big_file.test")
    #     self.client.get("/big_file.test")

    @task
    def bad_page(self):
        with self.client.get("/non-existent.url", catch_response=True) as response:
            if response.status_code == 404:
                response.success()
