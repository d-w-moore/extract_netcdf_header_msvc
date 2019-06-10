FROM python:2.7

RUN pip install netCDF4 numpy

ADD create_example_data.py /

RUN chmod +x /create_example_data.py

CMD [ "/create_example_data.py" ]
