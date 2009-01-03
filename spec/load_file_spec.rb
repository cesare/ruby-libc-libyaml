require File.join(File.dirname(__FILE__), 'spec_helper.rb')

describe YAML::LibYAML do
  it 'shoud raise exeption in unexist file.' do
    lambda {
      YAML::LibYAML.load_file('must_not_exist')
    }.should raise_error(Errno::ENOENT)
  end
end
